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
#include "esp_timer.h"
#include <optional>
//PIN in RC522
constexpr gpio_num_t PIN_RST  = GPIO_NUM_22;
constexpr gpio_num_t PIN_MOSI = GPIO_NUM_23;
constexpr gpio_num_t PIN_MISO = GPIO_NUM_19;
constexpr gpio_num_t PIN_SCK  = GPIO_NUM_18;
constexpr gpio_num_t PIN_SDA  = GPIO_NUM_5;


//RC522 Register
constexpr uint8_t REG_COMMAND      = 0x01;
constexpr uint8_t REG_COM_IEN      = 0x02;
constexpr uint8_t REG_DIV_IEN      = 0x03;
constexpr uint8_t REG_COM_IRQ      = 0x04;
constexpr uint8_t REG_FIFO_DATA    = 0x09;
constexpr uint8_t REG_FIFO_LEVEL   = 0x0A;
constexpr uint8_t REG_CONTROL      = 0x0C;
constexpr uint8_t REG_BIT_FRAMING  = 0x0D;
constexpr uint8_t REG_MODE         = 0x11;
constexpr uint8_t REG_TX_CONTROL   = 0x14;
constexpr uint8_t REG_TX_ASK       = 0x15;
constexpr uint8_t REG_VERSION      = 0x37;
constexpr uint8_t REG_ERROR        = 0x06;
constexpr uint8_t REG_COLL         = 0x0E;
constexpr uint8_t REG_CRC_RESULT_H = 0x21;
constexpr uint8_t REG_CRC_RESULT_L = 0x22;

//RC522 Command
constexpr uint8_t CMD_IDLE       = 0x00;
constexpr uint8_t CMD_TRANSCEIVE = 0x0C;
constexpr uint8_t CMD_SOFT_RESET = 0x0F;
constexpr uint8_t CMD_CALC_CRC   = 0x03;
constexpr uint8_t PICC_REQIDL    = 0x26;
constexpr uint8_t PICC_ANTICOLL  = 0x93;



class RC522 {
friend struct RC522TestAccess;

public:
    explicit RC522(spi_host_device_t host);
    ~RC522();

    RC522(const RC522&) = delete;
    RC522& operator=(const RC522&) = delete;
    
    esp_err_t init();
    esp_err_t antenna_on();
    esp_err_t antenna_off();
    esp_err_t read_version(uint8_t *out_version);
    esp_err_t read_uid(std::array<uint8_t, 4>& out_uid);
    esp_err_t deinit(bool free_bus = false);

private:
    spi_host_device_t host_;
    spi_device_handle_t spi_ = nullptr;


    //INFO
    esp_err_t transceive(const uint8_t* send_data, size_t send_len, uint8_t* recv_data, size_t* recv_len, uint8_t* out_valid_bits = nullptr);
    esp_err_t request(uint8_t req_mode, std::array<uint8_t, 2>& out_atqa);
    esp_err_t anticoll(std::array<uint8_t, 4>& out_uid);

    //ACTION WITH REGISTER
    esp_err_t write_reg(uint8_t addr, uint8_t val);
    esp_err_t read_reg(uint8_t addr, uint8_t *out_val);
    esp_err_t set_bit_mask(uint8_t addr, uint8_t mask);
    esp_err_t clear_bit_mask(uint8_t addr, uint8_t mask);
};



#endif