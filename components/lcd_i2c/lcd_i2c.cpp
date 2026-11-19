#include "lcd_i2c.hpp"
#include <cstring>

namespace {

std::array<uint8_t, 5> get_glyph_5x7(char c) {
    const unsigned char uc = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(c)));
    switch (uc) {
        case '0': return {0x3E, 0x41, 0x41, 0x41, 0x3E};
        case '1': return {0x00, 0x42, 0x7F, 0x40, 0x00};
        case '2': return {0x62, 0x51, 0x49, 0x49, 0x46};
        case '3': return {0x22, 0x41, 0x49, 0x49, 0x36};
        case '4': return {0x18, 0x14, 0x12, 0x7F, 0x10};
        case '5': return {0x27, 0x45, 0x45, 0x45, 0x39};
        case '6': return {0x3E, 0x49, 0x49, 0x49, 0x32};
        case '7': return {0x01, 0x71, 0x09, 0x05, 0x03};
        case '8': return {0x36, 0x49, 0x49, 0x49, 0x36};
        case '9': return {0x26, 0x49, 0x49, 0x49, 0x3E};
        case 'A': return {0x7E, 0x09, 0x09, 0x09, 0x7E};
        case 'B': return {0x7F, 0x49, 0x49, 0x49, 0x36};
        case 'C': return {0x3E, 0x41, 0x41, 0x41, 0x22};
        case 'D': return {0x7F, 0x41, 0x41, 0x41, 0x3E};
        case 'E': return {0x7F, 0x49, 0x49, 0x49, 0x41};
        case 'F': return {0x7F, 0x09, 0x09, 0x09, 0x01};
        case 'G': return {0x3E, 0x41, 0x49, 0x49, 0x3A};
        case 'H': return {0x7F, 0x08, 0x08, 0x08, 0x7F};
        case 'I': return {0x00, 0x41, 0x7F, 0x41, 0x00};
        case 'J': return {0x20, 0x40, 0x41, 0x3F, 0x01};
        case 'K': return {0x7F, 0x08, 0x14, 0x22, 0x41};
        case 'L': return {0x7F, 0x40, 0x40, 0x40, 0x40};
        case 'M': return {0x7F, 0x02, 0x0C, 0x02, 0x7F};
        case 'N': return {0x7F, 0x04, 0x08, 0x10, 0x7F};
        case 'O': return {0x3E, 0x41, 0x41, 0x41, 0x3E};
        case 'P': return {0x7F, 0x09, 0x09, 0x09, 0x06};
        case 'Q': return {0x3E, 0x41, 0x51, 0x21, 0x5E};
        case 'R': return {0x7F, 0x09, 0x19, 0x29, 0x46};
        case 'S': return {0x26, 0x49, 0x49, 0x49, 0x32};
        case 'T': return {0x01, 0x01, 0x7F, 0x01, 0x01};
        case 'U': return {0x3F, 0x40, 0x40, 0x40, 0x3F};
        case 'V': return {0x1F, 0x20, 0x40, 0x20, 0x1F};
        case 'W': return {0x7F, 0x20, 0x18, 0x20, 0x7F};
        case 'X': return {0x63, 0x14, 0x08, 0x14, 0x63};
        case 'Y': return {0x07, 0x08, 0x70, 0x08, 0x07};
        case 'Z': return {0x61, 0x51, 0x49, 0x45, 0x43};
        case ':': return {0x00, 0x00, 0x24, 0x00, 0x00};
        case '/': return {0x20, 0x10, 0x08, 0x04, 0x02};
        case '-': return {0x08, 0x08, 0x08, 0x08, 0x08};
        case '.': return {0x00, 0x00, 0x40, 0x00, 0x00};
        case '!': return {0x00, 0x00, 0x5F, 0x00, 0x00};
        case '_': return {0x40, 0x40, 0x40, 0x40, 0x40};
        default:  return {0x00, 0x00, 0x00, 0x00, 0x00};
    }
}

}

LCDI2C::LCDI2C(i2c_port_num_t port,
               gpio_num_t sda_pin,
               gpio_num_t sck_pin,
               uint8_t i2c_addr,
               uint8_t col_offset)
    : port_(port),
      sda_pin_(sda_pin),
      sck_pin_(sck_pin),
      addr_(i2c_addr),
      col_offset_(col_offset) {}

LCDI2C::~LCDI2C() {
    deinit(true);
}

esp_err_t LCDI2C::recover_bus() {
    if (dev_handle_ != nullptr) {
        i2c_master_bus_rm_device(dev_handle_);
        dev_handle_ = nullptr;
    }
    if (bus_handle_ != nullptr) {
        i2c_del_master_bus(bus_handle_);
        bus_handle_ = nullptr;
    }

    i2c_master_bus_config_t bus_cfg{};
    bus_cfg.i2c_port = port_;
    bus_cfg.sda_io_num = sda_pin_;
    bus_cfg.scl_io_num = sck_pin_;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_handle_);
    if (err != ESP_OK) {
        return err;
    }

    i2c_device_config_t dev_cfg{};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = addr_;
    dev_cfg.scl_speed_hz = 100000;
    dev_cfg.scl_wait_us = 10000;

    return i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle_);
}

esp_err_t LCDI2C::init() {
    if (dev_handle_ != nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = recover_bus();
    if (err != ESP_OK) {
        deinit(true);
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    auto check = [&](esp_err_t e) {
        if (e != ESP_OK) {
            deinit(true);
        }
        return e;
    };

    if (auto e = check(send_cmd(0xAE)); e != ESP_OK) return e;        // Display OFF
    if (auto e = check(send_cmd2(0xD5, 0x80)); e != ESP_OK) return e; // Set Display Clock Divide
    if (auto e = check(send_cmd2(0xA8, 0x3F)); e != ESP_OK) return e; // Set Multiplex Ratio (64 MUX)
    if (auto e = check(send_cmd2(0xD3, 0x00)); e != ESP_OK) return e; // Set Display Offset
    if (auto e = check(send_cmd(0x40)); e != ESP_OK) return e;        // Set Start Line
    if (auto e = check(send_cmd2(0x8D, 0x14)); e != ESP_OK) return e; // Enable Charge Pump
    if (auto e = check(send_cmd2(0x20, 0x02)); e != ESP_OK) return e; // Page Addressing Mode
    if (auto e = check(send_cmd(0xA1)); e != ESP_OK) return e;        // Segment Re-map
    if (auto e = check(send_cmd(0xC8)); e != ESP_OK) return e;        // COM Output Scan Direction
    if (auto e = check(send_cmd2(0xDA, 0x12)); e != ESP_OK) return e; // COM Pins Config
    if (auto e = check(send_cmd2(0x81, 0x7F)); e != ESP_OK) return e; // Contrast Control
    if (auto e = check(send_cmd2(0xD9, 0xF1)); e != ESP_OK) return e; // Pre-charge Period
    if (auto e = check(send_cmd2(0xDB, 0x40)); e != ESP_OK) return e; // VCOMH Deselect Level
    if (auto e = check(send_cmd(0x2E)); e != ESP_OK) return e;        // Deactivate Scroll
    if (auto e = check(send_cmd(0xA4)); e != ESP_OK) return e;        // Output follows RAM
    if (auto e = check(send_cmd(0xA6)); e != ESP_OK) return e;        // Normal Display
    if (auto e = check(send_cmd(0xAF)); e != ESP_OK) return e;        // Display ON

    if (auto e = check(clear()); e != ESP_OK) return e;
    return ESP_OK;
}

esp_err_t LCDI2C::deinit(bool free_bus) {
    esp_err_t first_err = ESP_OK;

    if (dev_handle_ != nullptr) {
        esp_err_t err = i2c_master_bus_rm_device(dev_handle_);
        dev_handle_ = nullptr;
        if (err != ESP_OK && first_err == ESP_OK) {
            first_err = err;
        }
    }

    if (free_bus && bus_handle_ != nullptr) {
        esp_err_t err = i2c_del_master_bus(bus_handle_);
        bus_handle_ = nullptr;
        if (err != ESP_OK && first_err == ESP_OK) {
            first_err = err;
        }
    }

    return first_err;
}

esp_err_t LCDI2C::transmit(const uint8_t* data, size_t size) {
    if (dev_handle_ == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = i2c_master_transmit(dev_handle_, data, size, I2C_TIMEOUT_MS);
    if (err == ESP_OK) {
        return ESP_OK;
    }

    recover_bus();
    vTaskDelay(pdMS_TO_TICKS(10));

    for (int attempt = 0; attempt < 6; ++attempt) {
        err = i2c_master_transmit(dev_handle_, data, size, I2C_TIMEOUT_MS);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return err;
}

esp_err_t LCDI2C::send_cmd(uint8_t cmd) {
    const uint8_t payload[2] = {0x00, cmd};
    return transmit(payload, sizeof(payload));
}

esp_err_t LCDI2C::send_cmd2(uint8_t cmd, uint8_t arg) {
    const uint8_t payload[3] = {0x00, cmd, arg};
    return transmit(payload, sizeof(payload));
}

esp_err_t LCDI2C::send_cmds(const uint8_t* cmds, size_t len) {
    if (len == 0 || len > 31) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t payload[32];
    payload[0] = 0x00;
    std::memcpy(&payload[1], cmds, len);
    return transmit(payload, len + 1);
}

esp_err_t LCDI2C::write_page(uint8_t page, const uint8_t data[WIDTH]) {
    if (dev_handle_ == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    if (page >= PAGES) {
        return ESP_ERR_INVALID_ARG;
    }

    const uint8_t set_addr[3] = {
        static_cast<uint8_t>(0xB0 | page),
        static_cast<uint8_t>(0x00 | (col_offset_ & 0x0F)),
        static_cast<uint8_t>(0x10 | ((col_offset_ >> 4) & 0x0F)),
    };
    esp_err_t err = send_cmds(set_addr, sizeof(set_addr));
    if (err != ESP_OK) {
        return err;
    }
    
    uint8_t payload[WIDTH + 1];
    payload[0] = 0x40;
    std::memcpy(&payload[1], data, WIDTH);
    return transmit(payload, sizeof(payload));
}

esp_err_t LCDI2C::clear() {
    uint8_t blank[WIDTH]{};
    for (uint8_t p = 0; p < PAGES; ++p) {
        esp_err_t err = write_page(p, blank);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

esp_err_t LCDI2C::set_display_on(bool on) {
    return send_cmd(on ? 0xAF : 0xAE);
}

esp_err_t LCDI2C::set_invert(bool invert) {
    return send_cmd(invert ? 0xA7 : 0xA6);
}

esp_err_t LCDI2C::print_line(uint8_t page, std::string_view text) {
    if (dev_handle_ == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    if (page >= PAGES) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t line_buf[WIDTH]{};
    constexpr uint8_t CHAR_STEP = 6;  // 5px chữ + 1px khoảng cách (tối đa 21 ký tự/dòng)
    const size_t max_chars = WIDTH / CHAR_STEP;

    for (size_t i = 0; i < text.size() && i < max_chars; ++i) {
        const auto glyph = get_glyph_5x7(text[i]);
        std::memcpy(&line_buf[i * CHAR_STEP], glyph.data(), glyph.size());
    }

    return write_page(page, line_buf);
}
