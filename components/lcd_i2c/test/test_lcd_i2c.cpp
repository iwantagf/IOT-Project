#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd_i2c.hpp"
#include "unity.h"


TEST_CASE("I2C full bus scan", "[lcd][hw]") {
    i2c_master_bus_config_t bus_cfg{};
    bus_cfg.i2c_port = I2C_NUM_0;
    bus_cfg.sda_io_num = GPIO_NUM_21;
    bus_cfg.scl_io_num = GPIO_NUM_22;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus = nullptr;
    TEST_ASSERT_EQUAL(ESP_OK, i2c_new_master_bus(&bus_cfg, &bus));

    int found = 0;
    printf("\n--- Quet I2C tren SDA=21, SCL=22 ---\n");
    for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
        if (i2c_master_probe(bus, addr, 50) == ESP_OK) {
            printf("Tim thay thiet bi tai: 0x%02X\n", addr);
            ++found;
        }
    }
    printf("--- Tong cong: %d thiet bi ---\n", found);

    i2c_del_master_bus(bus);
    TEST_ASSERT_GREATER_THAN(0, found);
}

TEST_CASE("OLED fails cleanly before init", "[oled]") {
    LCDI2C oled;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, oled.clear());
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, oled.print_line(0, "TEST"));
}

TEST_CASE("OLED full hardware lifecycle and display", "[oled][hw]") {
    LCDI2C oled(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 0x3C);

    // 1. Init & chặn gọi init lần 2
    TEST_ASSERT_EQUAL(ESP_OK, oled.init());
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, oled.init());

    // 2. Kiểm tra chặn vượt dòng (page 0..7)
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, oled.print_line(8, "OUT OF RANGE"));

    // 3. Hiển thị chữ bãi đỗ xe lên màn OLED
    TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(0, "SMART PARKING"));
    TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(2, "SLOTS FREE: 4/4"));
    TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(4, "SCAN RFID CARD"));
    TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(6, "STATUS: READY"));
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 4. Test đảo màu
    TEST_ASSERT_EQUAL(ESP_OK, oled.set_invert(true));
    vTaskDelay(pdMS_TO_TICKS(500));
    TEST_ASSERT_EQUAL(ESP_OK, oled.set_invert(false));

    // 5. Giải phóng sạch sẽ ở cuối
    TEST_ASSERT_EQUAL(ESP_OK, oled.deinit(true));
}

TEST_CASE("OLED display SMART PARKING for 10s", "[oled][hw]") {
    LCDI2C oled(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 0x3C);

    TEST_ASSERT_EQUAL(ESP_OK, oled.init());
    TEST_ASSERT_EQUAL(ESP_OK, oled.clear());
    TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(3, "    SMART PARKING    "));

    // Giữ màn hình hiển thị trong 10 giây (lặp lại định kỳ mỗi giây để bus luôn active)
    for (int i = 0; i < 10; ++i) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST_ASSERT_EQUAL(ESP_OK, oled.print_line(3, "    SMART PARKING    "));
    }

    TEST_ASSERT_EQUAL(ESP_OK, oled.deinit(true));
}
