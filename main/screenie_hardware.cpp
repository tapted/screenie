#include "screenie_hardware.hpp"

#include "espbase/main_loop.hpp"
#include "espbase/main_loop_task.hpp"
#include "espbase/nvs_store.hpp"
#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/config.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "halpp/rfid/pn532.hpp"

static MainLoopTask<int> rfid_poll;

static void on_tag_callback(void*, halpp::Pn532& pn532, std::span<const uint8_t> uid) {
  ESP_LOGI("APP", "Tag Scanned! Length: %d", uid.size());
  // When you're ready, tell it to scan again!
  pn532.start_passive_target_read();
}

static void setup_rfid() {
  halpp::Pn532::init_default(halpp::Pn532::I2C_ADDRESS_DEFAULT, GPIO_NUM_3)
      .log_error("screenie_hardware", "Failed to init default PN532");
  auto& pn532 = halpp::Pn532::default_instance();

  pn532.set_on_tag_callback(on_tag_callback);
//   rfid_poll.start(0, [](auto&) -> std::optional<uint32_t> {
//     halpp::Pn532::default_instance().poll();
//     return 1000;
//   });

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
