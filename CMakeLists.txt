if(ESP_PLATFORM)
idf_component_register(SRCS "ws2812.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver)
else()
    message(FATAL_ERROR "ESP_PLATFORM is not defined. Try reinstalling ESP-IDF.")
endif()
