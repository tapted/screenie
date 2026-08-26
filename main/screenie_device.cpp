#include "screenie_device.hpp"

#include <mqtt_client.h>

#include "espbase/main_loop_task.hpp"
#include "halpp/config.hpp"
#include "halpp/display/st7789.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "happy/entities/light.hpp"

static void on_light_update(const HAPPY::Entities::Light& light);
static void on_light_effect(const HAPPY::Entities::Light& light, std::string_view effect);
static void on_light_flash(const HAPPY::Entities::Light& light, std::string_view flash);

static void on_backlight_update(const HAPPY::Entities::Light& light);

static constexpr const char* led_effects[] = {"Rainbow", "Player Turn"};

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
  halpp::St7789::default_instance().set_backlight(light.is_on(), brightness);
}

struct RainbowData {
  uint16_t hue = 0;
};

struct FlashData {
  int toggles_left = 4;
  bool current_state = false;
  uint8_t r = 0, g = 0, b = 0;
};

static constinit MainLoopTask<RainbowData> rainbow_task;
static constinit RainbowData rainbow_data;

static constinit MainLoopTask<FlashData> flash_task;
static constinit FlashData flash_data;

static std::optional<uint32_t> rainbow_step(MainLoopTask<RainbowData>& task) {
  auto* data = task.data();
  auto& strip = halpp::LedStrip::default_instance();

  // Pass 65535 for 100% saturation and 100% brightness
  strip.set_pixel_hsv_16(0, data->hue, 65535, 65535);

  // Increment hue by a small amount and wrap cleanly at 360 degrees
  data->hue = (data->hue + 2) % 360;

  return 20;  // Run again in 20ms
}

static std::optional<uint32_t> flash_step(MainLoopTask<FlashData>& task) {
  auto* data = task.data();
  auto& strip = halpp::LedStrip::default_instance();

  if (data->toggles_left <= 0) {
    // Restore the final ON state before terminating
    strip.set_pixel(0, data->r, data->g, data->b);
    strip.refresh();
    return std::nullopt;  // End the task[cite: 5]
  }

  data->current_state = !data->current_state;
  if (data->current_state) {
    strip.set_pixel(0, data->r, data->g, data->b);
  } else {
    strip.set_pixel(0, 0, 0, 0);
  }

  strip.refresh();
  data->toggles_left--;

  return 250;  // Toggle every 250ms[cite: 5]
}

static void on_light_update(const HAPPY::Entities::Light& light) {
  rainbow_task.request_stop();
  flash_task.request_stop();
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
  if (effect == "Rainbow") {
    rainbow_task.start(&rainbow_data, rainbow_step);
  } else if (effect == "None") {
    rainbow_task.request_stop();
    // Force an immediate UI state sync to hardware
    on_light_update(light);
  }
}

static void on_light_flash(const HAPPY::Entities::Light& light, std::string_view flash) {
  ESP_LOGI("Light", "Flash command received: %.*s", static_cast<int>(flash.length()), flash.data());
  auto [r, g, b] = light.scaled_rgb();
  if (flash == "short") {
    flash_data = {4, false, r, g, b};
    // Ensure rainbow is temporarily halted if it was running, to prevent hardware conflicts
    rainbow_task.request_stop();
    flash_task.start(&flash_data, flash_step);
  }
}