idf_component_register(
    SRCS "lcd_i2c.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_driver_i2c esp_driver_gpio esp_timer freertos
)