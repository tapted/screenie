#include "screenie_device.hpp"

#include <mqtt_client.h>

#include "halpp/display/display.hpp"
#include "halpp/led_strip/led_effects.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "happy/entities/light.hpp"

static void on_light_update(const HAPPY::Entities::Light& light);
static void on_light_effect(const HAPPY::Entities::Light& light, std::string_view effect);
static void on_light_flash(const HAPPY::Entities::Light& light, std::string_view flash);

static void on_backlight_update(const HAPPY::Entities::Light& light);

static constexpr const char* led_effects[] = {"Rainbow", "Breathe", "Player Turn"};

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
                                              .effect_list = led_effects,
                                              .on_effect = on_light_effect,
                                              .on_flash = on_light_flash,
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

static void on_backlight_update(const HAPPY::Entities::Light& light) {
  uint8_t brightness = static_cast<uint32_t>(light.brightness()) * 100 / 255;
  halpp::Display::instance().set_backlight(
      light.is_on() ? halpp::BacklightState::On : halpp::BacklightState::Off, brightness);
}

static void on_light_update(const HAPPY::Entities::Light& light) {
  halpp::stop_led_effects();
  if (!light.is_on()) {
    halpp::LedStrip::default_instance().clear();
    return;
  }
  auto [r, g, b] = light.scaled_rgb();
  halpp::LedStrip::default_instance().set_pixel(0, r, g, b);
}

static void on_light_effect(const HAPPY::Entities::Light& light, std::string_view effect) {
  ESP_LOGI("Light", "Effect command received: %.*s", static_cast<int>(effect.length()),
           effect.data());
  auto [r, g, b] = light.scaled_rgb();
  if (effect == "Rainbow") {
    halpp::start_led_rainbow();
  } else if (effect == "Player Turn") {
    bool end_in_on_state = light.is_on();
    halpp::start_led_flash(r, g, b, end_in_on_state);
  } else if (effect == "Breathe") {
    halpp::start_led_breathe(halpp::rgb_to_hue(r, g, b));
  } else if (effect == "None") {
    halpp::stop_led_effects();
    // Force an immediate UI state sync to hardware
    on_light_update(light);
  }
}

static void on_light_flash(const HAPPY::Entities::Light& light, std::string_view flash) {
  ESP_LOGI("Light", "Flash command received: %.*s", static_cast<int>(flash.length()), flash.data());
  auto [r, g, b] = light.scaled_rgb();
  if (flash == "short") {
    halpp::start_led_flash(r, g, b);
  }
}