#include <cstdint>
#include "unity.h"
#include "esp_err.h"
#include "rfid_rc522.hpp"

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

namespace {

RC522& get_rfid() {
    static RC522 rfid(SPI2_HOST);
    static bool initialized = false;
    if (!initialized) {
        TEST_ASSERT_EQUAL(ESP_OK, rfid.init());
        initialized = true;
    }
    return rfid;
}

}  // namespace

TEST_CASE("RC522 fails cleanly before init", "[rfid][unit]") {
    RC522 uninit_rfid(SPI2_HOST);
    uint8_t val = 0;

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE,
                      RC522TestAccess::write_reg(uninit_rfid, REG_MODE, 0x3D));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE,
                      RC522TestAccess::read_reg(uninit_rfid, REG_VERSION, &val));
}

TEST_CASE("RC522 rejects null output pointer", "[rfid][unit]") {
    RC522& rfid = get_rfid();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      RC522TestAccess::read_reg(rfid, REG_VERSION, nullptr));
}

TEST_CASE("RC522 init and VersionReg check", "[rfid][hw]") {
    RC522& rfid = get_rfid();
    uint8_t version = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.read_version(&version));
    printf("RC522 VersionReg = 0x%02X\n", version);
    TEST_ASSERT_TRUE_MESSAGE(
        version == 0x91 || version == 0x92 || version == 0x82,
        "VersionReg invalid (check SPI / 3.3V / RST)"
    );
}

TEST_CASE("RC522 antenna_off and antenna_on toggle TxControlReg bits", "[rfid][hw]") {
    RC522& rfid = get_rfid();
    uint8_t tx_ctrl = 0;

    TEST_ASSERT_EQUAL(ESP_OK, rfid.antenna_off());
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &tx_ctrl));
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_ctrl & 0x03);

    TEST_ASSERT_EQUAL(ESP_OK, rfid.antenna_on());
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &tx_ctrl));
    TEST_ASSERT_EQUAL_HEX8(0x03, tx_ctrl & 0x03);
}

TEST_CASE("RC522 set_bit and clear_bit preserve other bits", "[rfid][hw]") {
    RC522& rfid = get_rfid();
    uint8_t before = 0;
    uint8_t after = 0;

    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &before));

    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::clear_bit(rfid, REG_TX_CONTROL, 0x03));
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &after));
    TEST_ASSERT_EQUAL_HEX8(static_cast<uint8_t>(before & ~0x03), after);

    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::set_bit(rfid, REG_TX_CONTROL, 0x03));
    TEST_ASSERT_EQUAL(ESP_OK, RC522TestAccess::read_reg(rfid, REG_TX_CONTROL, &after));
    TEST_ASSERT_EQUAL_HEX8(static_cast<uint8_t>(before | 0x03), after);
}