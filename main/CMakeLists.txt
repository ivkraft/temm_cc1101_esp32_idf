idf_component_register(SRCS "app_main.c"
                       REQUIRES board display cc1101 gfx esp_timer input SPI decoder
                       INCLUDE_DIRS ".")
spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)
