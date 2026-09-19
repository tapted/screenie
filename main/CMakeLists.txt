set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE DEPS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/display/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/gpio/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/i2c/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/ledc/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/led_strip/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/network/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/buzzer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/halpp/rfid/pn532.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/happy/src/*.cpp
)

file(GLOB_RECURSE MAIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp
)

set(PROJECT_SOURCES ${MAIN_SRCS} ${DEPS_SRCS})
set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/happy/src)

idf_component_register(
    SRCS ${PROJECT_SOURCES}
    INCLUDE_DIRS "." ${PROJECT_INCLUDE_DIRS}
    REQUIRES espbase led_strip esp_hw_support esp_driver_ledc esp_netif esp_wifi esp_lcd esp_https_ota esp_driver_tsens wpa_supplicant esp_driver_i2s)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -W -Wextra -Wunused -fno-use-cxa-atexit)
