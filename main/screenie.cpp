#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include "halpp/config.hpp"
#include "halpp/led_strip/led_strip.hpp"

using halpp::LedStrip;

static constexpr const char TAG[] = "screenie";

extern "C" void app_main(void) {
  LedStrip::init_default({
                             .gpio_num = halpp::config::IndicatorLed::PIN_RGB,
                             .color_fmt = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
                             .auto_refresh = true,
                         })
      .log_error(TAG, "LedStrip init");
  LedStrip::default_instance().set_pixel(0, 100, 0, 100).log_error(TAG, "LedStrip set_pixel");

  ESP_LOGI(TAG, "Hello, world 2!");
}