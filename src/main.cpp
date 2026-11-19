#include <array>
#include <cstdio>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

constexpr std::array<gpio_num_t, 4> LED_PINS = {
    GPIO_NUM_13,
    GPIO_NUM_14,
    GPIO_NUM_26,
    GPIO_NUM_33,
};

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
constexpr uint8_t CMD_IDLE       = 0x00;
constexpr uint8_t CMD_TRANSCEIVE = 0x0C;
constexpr uint8_t CMD_SOFT_RESET = 0x0F;
constexpr uint8_t PICC_REQIDL    = 0x26;
constexpr uint8_t PICC_ANTICOLL  = 0x93;

class RC522 {
public:
    explicit RC522(spi_host_device_t host): host_(host) {};
    
    esp_err_t set_bit_mask(uint8_t addr, uint8_t mask) {
        uint8_t value;
        esp_err_t err = read_reg(addr, &value);
        if (err != ESP_OK)
            return err;

        value |= mask;
        
        return write_reg(addr, value);
    }

    esp_err_t clear_bit_mask(uint8_t addr, uint8_t mask) {
        uint8_t value;
        esp_err_t err = read_reg(addr, &value);

        if (err != ESP_OK)
            return err;

        value &= ~mask;

        return write_reg(addr, value);
    }

private:
    spi_host_device_t host_;
    spi_device_handle_t spi_ = nullptr;

    esp_err_t write_reg(uint8_t addr, uint8_t val) {
        if (spi_ == nullptr) {
            std::cout << "SPI is NULL" << std::endl;

            return ESP_ERR_INVALID_STATE;
        }

        uint8_t tx[2] = {static_cast<uint8_t> ((addr << 1) & 0x7E), val};
        spi_transaction_t t{};

        t.length = 16;
        t.tx_buffer = tx;
        spi_device_transmit(spi_, &t);
    }

    esp_err_t read_reg(uint8_t addr, uint8_t *out_val) {
        if (spi_ == nullptr) {
            std::cout << "SPI is NULL" << std::endl;

            return ESP_ERR_INVALID_STATE;
        }

        if (out_val == nullptr) {
            std::cout << "Can not write to nullptr" << std::endl;
            return ESP_ERR_INVALID_STATE;
        }

        uint8_t tx[2] = {static_cast<uint8_t>(((addr << 1) & 0x7E) | 0x80), 0x00};
        uint8_t rx[2]{0, 0};
        spi_transaction_t t{};
        
        t.length = 16;
        t.tx_buffer = tx;
        t.rx_buffer = rx;
        
        esp_err_t err = spi_device_transmit(spi_, &t);
        
        if (err != ESP_OK) {
            return err;
        }

        *out_val = rx[1];

        return ESP_OK;
    }
};

extern "C" void app_main(void) {

}