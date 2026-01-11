idf_component_register(
    SRCS "decoder.c"
    INCLUDE_DIRS "include"
    REQUIRES driver board esp_timer freertos
)