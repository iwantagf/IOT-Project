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


extern "C" void app_main(void) {

}