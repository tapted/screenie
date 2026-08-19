#include "screenie_device.hpp"

#include <mqtt_client.h>

#include "halpp/led_strip/led_strip.hpp"
#include "happy/entities/light.hpp"

static void on_light_update(const HAPPY::Entities::Light& light);

constinit HAPPY::Transports::MqttDevice screenie_device({
    .identifiers = "screenie",
    .name = "Screenie",
    .manufacturer = "Waveshare",
    .model = "ESP32-C6 LCD 1.47",
    .append_mac_chars = 4,  // Append last 4 chars of MAC to identifiers and name
});

HAPPY::Entities::Light onboard_led(screenie_device, "status_led", "Onboard LED",
                                   {
                                       .supports_rgb = true,
                                       .on_update = on_light_update,
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
  auto [r, g, b] = light.scaled_rgb();
  halpp::LedStrip::default_instance().set_pixel(0, r, g, b);
}
