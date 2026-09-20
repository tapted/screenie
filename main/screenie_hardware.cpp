#include "screenie_hardware.hpp"

#include <vector>

#include "espbase/main_loop.hpp"
#include "espbase/nvs_store.hpp"
#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/config.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "halpp/rfid/pn532.hpp"

static std::vector<uint8_t> last_uid;
static int64_t last_scan_time_us = 0;
constexpr int64_t DEBOUNCE_TIMEOUT_US = 3000000;  // 3 seconds

static void on_tag_callback(void*, halpp::Pn532& pn532, std::span<const uint8_t> uid) {
  int64_t now = esp_timer_get_time();
  bool is_same_tag = std::ranges::equal(uid, last_uid);
  bool is_within_timeout = (now - last_scan_time_us) < DEBOUNCE_TIMEOUT_US;

  if (is_same_tag && is_within_timeout) {
    ESP_LOGD("APP", "Ignored: Same tag scanned within debounce window.");
  } else {
    // Update the cache
    last_uid.assign(uid.begin(), uid.end());
    last_scan_time_us = now;
    uint64_t tag_id = 0;
    for (size_t i = 0; i < uid.size(); ++i) {
      tag_id = (tag_id << 8) | uid[i];
    }

    ESP_LOGI("APP", "Tag Scanned! Length: %d, ID: %016" PRIX64, uid.size(), tag_id);
  }
  main_loop.post_delayed<&halpp::Pn532::start_passive_target_read>(1000, &pn532);
}

static void setup_rfid() {
  halpp::Pn532::init_default(halpp::Pn532::I2C_ADDRESS_DEFAULT, GPIO_NUM_3, GPIO_NUM_4)
      .log_error("screenie_hardware", "Failed to init default PN532");
  auto& pn532 = halpp::Pn532::default_instance();

  pn532.set_on_tag_callback(on_tag_callback);

  std::array<uint8_t, 4> firmware_version;
  if (EspError err = pn532.get_firmware_version(firmware_version)) {
    err.log("screenie_hardware", "Failed to get PN532 firmware version");
  } else {
    ESP_LOGI("screenie_hardware", "Found PN5%X Firmware v%d.%d (Support Mask: 0x%02X)",
             firmware_version[0], firmware_version[1], firmware_version[2], firmware_version[3]);
  }

  // Initiate the very first hardware scan
  pn532.start_passive_target_read();
}

void init_screenie_hardware() {
  NvsStore::init_flash().log_error("screenie_hardware", "Failed to init NVS flash");
  halpp::LedStrip::init_default({
                                    .gpio_num = halpp::config::IndicatorLed::PIN_RGB,
                                    .color_fmt = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
                                    .auto_refresh = true,
                                })
      .log_error("screenie_hardware", "LedStrip init");
  halpp::Passive::Config buzzer_config = {.gpio_num = halpp::config::Buzzer::PIN_PWM};
  halpp::Passive::init_default(buzzer_config).log_error("screenie_hardware", "Passive Buzzer init");

  setup_rfid();

  main_loop.push_func(
      [](void*) { halpp::Passive::default_instance().play(halpp::beeps::acknowledge); });
}
