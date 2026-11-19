#include "rfid_rc522.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

RC522::RC522(spi_host_device_t host) : host_(host) {}

esp_err_t RC522::init() {
    // RST
    gpio_config_t io_conf{};
    io_conf.pin_bit_mask = (1ULL << PIN_RST);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        return err;
    }

    // SPI
    spi_bus_config_t bus_cfg{};
    bus_cfg.miso_io_num = PIN_MISO;
    bus_cfg.mosi_io_num = PIN_MOSI;
    bus_cfg.sclk_io_num = PIN_SCK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;

    err = spi_bus_initialize(host_, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    // Bind RC522 to SPI
    spi_device_interface_config_t dev_cfg{};
    dev_cfg.clock_speed_hz = 1 * 1000 * 1000;  // 1 MHz
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = PIN_SDA;
    dev_cfg.queue_size = 1;

    err = spi_bus_add_device(host_, &dev_cfg, &spi_);
    if (err != ESP_OK) {
        return err;
    }

    // Hard Reset with RST
    err = gpio_set_level(PIN_RST, 0);
    if (err != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(10));

    err = gpio_set_level(PIN_RST, 1);
    if (err != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(50));

    // Soft Reset and Register configuration
    err = write_reg(REG_COMMAND, CMD_SOFT_RESET);
    if (err != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(50));

    err = write_reg(REG_MODE, 0x3D);
    if (err != ESP_OK) return err;

    err = write_reg(REG_TX_ASK, 0x40);
    if (err != ESP_OK) return err;

    return antenna_on();
}

esp_err_t RC522::antenna_on() {
    uint8_t value = 0;

    esp_err_t err = read_reg(REG_TX_CONTROL, &value);

    if (err != ESP_OK) {
        return err;
    }

    if ((value & 0x03) != 0x03) {
        return set_bit_mask(REG_TX_CONTROL, 0x03);
    }

    return ESP_OK;
}

esp_err_t RC522::antenna_off() {
    uint8_t value = 0;

    esp_err_t err = read_reg(REG_TX_CONTROL, &value);

    if (err != ESP_OK) {
        return err;
    }

    if ((value & 0x03) != 0) {
        return clear_bit_mask(REG_TX_CONTROL, 0x03);
    }

    return ESP_OK;
}

esp_err_t RC522::write_reg(uint8_t addr, uint8_t val) {
    if (spi_ == nullptr) {
        std::cout << "SPI is NULL" << std::endl;

        return ESP_ERR_INVALID_STATE;
    }

    uint8_t tx[2] = {static_cast<uint8_t> ((addr << 1) & 0x7E), val};
    spi_transaction_t t{};

    t.length = 16;
    t.tx_buffer = tx;
    return spi_device_transmit(spi_, &t);
}

esp_err_t RC522::read_reg(uint8_t addr, uint8_t *out_val) {
    if (spi_ == nullptr) {
        std::cout << "SPI is NULL" << std::endl;

        return ESP_ERR_INVALID_STATE;
    }

    if (out_val == nullptr) {
        std::cout << "Can not write to nullptr" << std::endl;
        return ESP_ERR_INVALID_ARG;
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

esp_err_t RC522::set_bit_mask(uint8_t addr, uint8_t mask) {
    uint8_t value;
    esp_err_t err = read_reg(addr, &value);
    if (err != ESP_OK)
        return err;

    value |= mask;
    
    return write_reg(addr, value);
}

esp_err_t RC522::clear_bit_mask(uint8_t addr, uint8_t mask) {
    uint8_t value;
    esp_err_t err = read_reg(addr, &value);

    if (err != ESP_OK)
        return err;

    value &= ~mask;

    return write_reg(addr, value);
}

esp_err_t RC522::read_version(uint8_t *out_version) {
    return read_reg(REG_VERSION, out_version);
}