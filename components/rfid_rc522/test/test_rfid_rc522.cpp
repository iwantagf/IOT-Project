#include <array>
#include <cstdint>
#include <cstdio>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rfid_rc522.hpp"
#include "unity.h"

struct RC522TestAccess {
    static esp_err_t write_reg(RC522& r, uint8_t addr, uint8_t val) {
        return r.write_reg(addr, val);
    }
    static esp_err_t read_reg(RC522& r, uint8_t addr, uint8_t* out_val) {
        return r.read_reg(addr, out_val);
    }
    static esp_err_t set_bit(RC522& r, uint8_t addr, uint8_t mask) {
        return r.set_bit_mask(addr, mask);
    }
    static esp_err_t clear_bit(RC522& r, uint8_t addr, uint8_t mask) {
        return r.clear_bit_mask(addr, mask);
    }
};

TEST_CASE("RC522 fails cleanly before init", "[rfid][unit]") {
    RC522 rfid(SPI2_HOST);
    uint8_t val = 0;
    std::array<uint8_t, 4> uid{0xAA, 0xBB, 0xCC, 0xDD};

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE,
                      RC522TestAccess::write_reg(rfid, REG_MODE, 0x3D));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE,
                      RC522TestAccess::read_reg(rfid, REG_VERSION, &val));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, rfid.read_uid(uid));
}

TEST_CASE("RC522 rejects null output pointer", "[rfid][unit]") {
    RC522 rfid(SPI2_HOST);
    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      RC522TestAccess::read_reg(rfid, REG_VERSION, nullptr));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, rfid.read_version(nullptr));
}

TEST_CASE("RC522 init and deinit lifecycle", "[rfid][hw]") {
    RC522 rfid(SPI2_HOST);
    uint8_t version = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());
    TEST_ASSERT_EQUAL(ESP_OK, rfid.read_version(&version));

    TEST_ASSERT_EQUAL(ESP_OK, rfid.deinit());
    TEST_ASSERT_EQUAL(ESP_OK, rfid.deinit());

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, rfid.read_version(&version));
}

TEST_CASE("RC522 VersionReg check", "[rfid][hw]") {
    RC522 rfid(SPI2_HOST);
    uint8_t version = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());
    TEST_ASSERT_EQUAL(ESP_OK, rfid.read_version(&version));
    printf("RC522 VersionReg = 0x%02X\n", version);

    TEST_ASSERT_NOT_EQUAL_HEX8_MESSAGE(0x00, version, "SPI error (0x00)");
    TEST_ASSERT_NOT_EQUAL_HEX8_MESSAGE(0xFF, version, "SPI error (0xFF)");
}

TEST_CASE("RC522 antenna_off and antenna_on toggle TxControlReg bits", "[rfid][hw]") {
    RC522 rfid(SPI2_HOST);
    uint8_t tx_ctrl = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());

    TEST_ASSERT_EQUAL(ESP_OK, rfid.antenna_off());
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &tx_ctrl));
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_ctrl & 0x03);

    TEST_ASSERT_EQUAL(ESP_OK, rfid.antenna_on());
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &tx_ctrl));
    TEST_ASSERT_EQUAL_HEX8(0x03, tx_ctrl & 0x03);
}

TEST_CASE("RC522 set_bit and clear_bit preserve other bits", "[rfid][hw]") {
    RC522 rfid(SPI2_HOST);
    uint8_t before = 0;
    uint8_t after = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &before));

    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::clear_bit(rfid, REG_TX_CONTROL, 0x03));
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &after));
    TEST_ASSERT_EQUAL_HEX8(static_cast<uint8_t>(before & ~0x03), after);

    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::set_bit(rfid, REG_TX_CONTROL, 0x03));
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &after));
    TEST_ASSERT_EQUAL_HEX8(static_cast<uint8_t>(before | 0x03), after);
}

TEST_CASE("RC522 read_uid returns NOT_FOUND and clears buffer when no card", "[rfid][hw]") {
    RC522 rfid(SPI2_HOST);
    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());

    std::array<uint8_t, 4> uid{0xDE, 0xAD, 0xBE, 0xEF};
    constexpr std::array<uint8_t, 4> EMPTY_UID{0, 0, 0, 0};

    esp_err_t err = rfid.read_uid(uid);
    TEST_ASSERT_EQUAL(ESP_ERR_NOT_FOUND, err);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(EMPTY_UID.data(), uid.data(), uid.size());
}

TEST_CASE("RC522 read_uid reads real card UID within 10s", "[rfid][card]") {
    RC522 rfid(SPI2_HOST);
    TEST_ASSERT_EQUAL(ESP_OK, rfid.init());

    printf("\n>>> Hay quet the RFID trong vong 10 giay...\n");

    std::array<uint8_t, 4> uid{};
    constexpr std::array<uint8_t, 4> EMPTY_UID{0, 0, 0, 0};
    constexpr int64_t WAIT_US = 10LL * 1000 * 1000;
    const int64_t start_us = esp_timer_get_time();

    esp_err_t err = ESP_ERR_NOT_FOUND;
    while ((esp_timer_get_time() - start_us) < WAIT_US) {
        err = rfid.read_uid(uid);
        if (err == ESP_OK) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, err, "Het 10s khong doc duoc the RFID");
    TEST_ASSERT_TRUE_MESSAGE(uid != EMPTY_UID, "UID doc ra bi toan 0");

    printf(">>> Doc thanh cong! UID = %02X:%02X:%02X:%02X\n",
           uid[0], uid[1], uid[2], uid[3]);
}