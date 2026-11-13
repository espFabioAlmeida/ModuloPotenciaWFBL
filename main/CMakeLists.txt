idf_component_register(SRCS "Utils.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt nvs_flash esp_driver_ledc)
