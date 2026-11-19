idf_component_register(
    SRCS "rfid_rc522.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_driver_spi esp_driver_gpio freertos
)