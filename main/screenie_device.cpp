#include "screenie_device.hpp"

#include <mqtt_client.h>

#include "halpp/config.hpp"
#include "halpp/display/st7789.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "happy/entities/light.hpp"

static void on_light_update(const HAPPY::Entities::Light& light);
static void on_backlight_update(const HAPPY::Entities::Light& light);

constinit HAPPY::Transports::MqttDevice screenie_device({
    .identifiers = "screenie",
    .name = "Screenie",
    .manufacturer = "Waveshare",
    .model = "ESP32-C6 LCD 1.47",
    .append_mac_chars = 4,  // Append last 4 chars of MAC to identifiers and name
});

static HAPPY::Entities::Light onboard_led(screenie_device, "status_led", "Onboard LED",
                                          {
                                              .supports_rgb = true,
                                              .on_update = on_light_update,
                                          });

static HAPPY::Entities::Light screen_backlight(screenie_device, "screen_backlight",
                                               "Screen Backlight",
                                               {
                                                   .icon = "mdi:brightness-5",
                                                   .supports_rgb = false,
                                                   .on_update = on_backlight_update,
                                               });

EspResult<> screenie_device_begin() {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.uri = "mqtt://10.1.0.201";
  // Cap the outbox to 8KB. If it fills up, enqueue will fail safely instead of OOMing.
  mqtt_cfg.outbox.limit = 8192;
  mqtt_cfg.credentials.username = "puck1e80";
  mqtt_cfg.credentials.authentication.password = "A9CeSm4MX7tcSMT";
  return screenie_device.begin(mqtt_cfg);
}

static void on_light_update(const HAPPY::Entities::Light& light) {
  if (!light.is_on() && !halpp::LedStrip::is_default_initialized()) {
    return;  // Don't try to update the LED if it's not initialized yet.
  }
  if (!light.is_on()) {
    halpp::LedStrip::deinit_default();
    return;
  }
  if (!halpp::LedStrip::is_default_initialized()) {
    halpp::LedStrip::init_default({
        .gpio_num = halpp::config::IndicatorLed::PIN_RGB,
        .color_fmt = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .auto_refresh = true,
    });
  }
  auto [r, g, b] = light.scaled_rgb();
  halpp::LedStrip::default_instance().set_pixel(0, r, g, b);
}

static void on_backlight_update(const HAPPY::Entities::Light& light) {
  uint8_t brightness = static_cast<uint32_t>(light.brightness()) * 100 / 255;
  halpp::St7789::default_instance().set_backlight(light.is_on(), brightness);
}