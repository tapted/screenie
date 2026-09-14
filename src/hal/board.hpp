#pragma once

#include <cstdint>
#include <esp_lcd_panel_st7789.h>

#include "halpp/config_defaults.hpp"

namespace halpp::board {

struct config : detail::Defaults {
  struct SpiBus : detail::Defaults::SpiBus {
    static constexpr uint32_t SPI_CLK_WRITE_HZ = 40 * 1000 * 1000;  // 40MHz for write
  };
  struct Display : detail::Defaults::Display {
    static constexpr uint16_t WIDTH = 172;
    static constexpr uint16_t HEIGHT = 320;
    static constexpr uint16_t X_GAP = 34;  // The 172x320 Magic Offset
    static constexpr lcd_rgb_element_order_t RGB_ELEMENT_ORDER = LCD_RGB_ELEMENT_ORDER_BGR;
    static constexpr lcd_rgb_data_endian_t DATA_ENDIAN = LCD_RGB_DATA_ENDIAN_LITTLE;
    static constexpr bool INVERT_COLORS = true;
    static constexpr uint8_t BACKLIGHT_DEFAULT = 30;

    static constexpr auto NEW_PANEL_FUNC = esp_lcd_new_panel_st7789;
  };
  struct lvgl : detail::Defaults::lvgl {
    static constexpr uint32_t BUFFER_FRACTION = 10;
  };
  struct Buzzer : detail::Defaults::Buzzer {
    static constexpr gpio_num_t PIN_PWM = GPIO_NUM_20;  // PWM output for passive buzzer
  };
};  // struct config

}  // namespace halpp::board
