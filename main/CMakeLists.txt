idf_component_register(SRCS "app_main.c"
                       REQUIRES board display cc1101 gfx esp_timer input SPI
                       INCLUDE_DIRS ".")
