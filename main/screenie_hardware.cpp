#include "screenie_hardware.hpp"

#include "espbase/nvs_store.hpp"
#include "halpp/config.hpp"
#include "halpp/led_strip/led_strip.hpp"

void init_screenie_hardware() {
  NvsStore::init_flash().log_error("screenie_hardware", "Failed to init NVS flash");
  halpp::LedStrip::init_default({
                                    .gpio_num = halpp::config::IndicatorLed::PIN_RGB,
                                    .color_fmt = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
                                    .auto_refresh = true,
                                })
      .log_error("screenie_hardware", "LedStrip init");
}
