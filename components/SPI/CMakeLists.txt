idf_component_register(
    SRCS "shared_spi.c"
    INCLUDE_DIRS "include"
    REQUIRES driver board esp_timer freertos
)
