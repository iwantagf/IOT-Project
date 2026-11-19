#ifndef LCD_I2C_HPP
#define LCD_I2C_HPP
#include <cstdint>
#include <array>
#include <cstdio>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_timer.h"
#include <string_view>


//Actually OLEDSSD1306 but i have named it like that T_T
class LCDI2C {
public:
    static constexpr uint8_t WIDTH = 128;
    static constexpr uint8_t HEIGHT = 64;
    static constexpr uint8_t PAGES = HEIGHT / 8;

    explicit LCDI2C(i2c_port_num_t port = I2C_NUM_0, 
        gpio_num_t sda_pin = GPIO_NUM_21, 
        gpio_num_t sck_pin = GPIO_NUM_22, 
        uint8_t i2c_addr = 0x3C, uint8_t col_offset = 0);

    ~LCDI2C();

    LCDI2C(const LCDI2C&) = delete;
    LCDI2C& operator=(const LCDI2C&) = delete;

    esp_err_t init();    
    esp_err_t deinit(bool free_bus = true);
    
    esp_err_t clear();
    esp_err_t set_display_on(bool on);
    esp_err_t set_invert(bool invert);
    esp_err_t print_line(uint8_t page, std::string_view text);


private:
    static constexpr int I2C_TIMEOUT_MS = 100;

    esp_err_t recover_bus();
    esp_err_t transmit(const uint8_t* data, size_t size);
    esp_err_t send_cmd(uint8_t cmd);
    esp_err_t send_cmd2(uint8_t cmd, uint8_t arg);
    esp_err_t send_cmds(const uint8_t* cmds, size_t len);
    esp_err_t write_page(uint8_t page, const uint8_t data[WIDTH]);

    i2c_port_num_t port_;
    gpio_num_t sda_pin_;
    gpio_num_t sck_pin_;
    uint8_t addr_;
    uint8_t col_offset_;

    i2c_master_bus_handle_t bus_handle_ = nullptr;
    i2c_master_dev_handle_t dev_handle_ = nullptr;
};


#endif