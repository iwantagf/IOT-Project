#ifndef RFID_RC522_HPP
#define RFID_RC522_HPP
#include <array>
#include <cstdio>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
//PIN in RC522
constexpr gpio_num_t PIN_RST = GPIO_NUM_22;
constexpr gpio_num_t PIN_MOSI = GPIO_NUM_23;
constexpr gpio_num_t PIN_MISO = GPIO_NUM_19;
constexpr gpio_num_t PIN_SCK = GPIO_NUM_18;
constexpr gpio_num_t PIN_SDA = GPIO_NUM_5;


//RC522 Register
constexpr uint8_t REG_COMMAND     = 0x01;
constexpr uint8_t REG_COM_IEN     = 0x02;
constexpr uint8_t REG_DIV_IEN     = 0x03;
constexpr uint8_t REG_COM_IRQ     = 0x04;
constexpr uint8_t REG_FIFO_DATA   = 0x09;
constexpr uint8_t REG_FIFO_LEVEL  = 0x0A;
constexpr uint8_t REG_CONTROL     = 0x0C;
constexpr uint8_t REG_BIT_FRAMING = 0x0D;
constexpr uint8_t REG_MODE        = 0x11;
constexpr uint8_t REG_TX_CONTROL  = 0x14;
constexpr uint8_t REG_TX_ASK      = 0x15;
constexpr uint8_t REG_VERSION     = 0x37;

//RC522 Command
constexpr uint8_t CMD_IDLE       = 0x00;
constexpr uint8_t CMD_TRANSCEIVE = 0x0C;
constexpr uint8_t CMD_SOFT_RESET = 0x0F;
constexpr uint8_t PICC_REQIDL    = 0x26;
constexpr uint8_t PICC_ANTICOLL  = 0x93;



class RC522 {
friend struct RC522TestAccess;

public:
    explicit RC522(spi_host_device_t host);
    
    esp_err_t init();
    esp_err_t antenna_on();
    esp_err_t antenna_off();
    esp_err_t read_version(uint8_t *out_version);

private:
    spi_host_device_t host_;
    spi_device_handle_t spi_ = nullptr;

    esp_err_t write_reg(uint8_t addr, uint8_t val);
    esp_err_t read_reg(uint8_t addr, uint8_t *out_val);
    esp_err_t set_bit_mask(uint8_t addr, uint8_t mask);
    esp_err_t clear_bit_mask(uint8_t addr, uint8_t mask);
};



#endif