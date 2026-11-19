#include <array>
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

constexpr std::array<gpio_num_t, 4> LED_PINS = {
    GPIO_NUM_13,
    GPIO_NUM_14,
    GPIO_NUM_26,
    GPIO_NUM_33,
};

extern "C" void app_main(void) {
    for (gpio_num_t pin : LED_PINS) {
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 0);
    }

    while (true) {
        for (int i = 0; i < 2; ++i) {
            gpio_num_t pin0 = LED_PINS[i];
            gpio_num_t pin1 = LED_PINS[3 - i];
            gpio_set_level(pin0, 1);
            gpio_set_level(pin1, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        std::printf("Đã BẬT 4 bóng LED\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        for (int i = 1; i > -1; --i) {
            gpio_num_t pin0 = LED_PINS[i];
            gpio_num_t pin1 = LED_PINS[3 - i];
            gpio_set_level(pin0, 0);
            gpio_set_level(pin1, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        std::printf("Đã TẮT 4 bóng LED\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}