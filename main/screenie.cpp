#include "espbase/boot/check_crash_loop.hpp"
#include "espbase/boot/delayed_pm_enable.hpp"
#include "espbase/boot/network_logger.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/main_loop.hpp"
#include "screenie_network.hpp"
#include "screenie_sensors.hpp"

static constexpr const char TAG[] = "screenie";

extern size_t system_diagnostic_free_iram_at_boot;

extern "C" void app_main(void) {
  check_crash_loop();
  system_diagnostic_free_iram_at_boot = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  initialize_network_logger(4096, false);
  // delayed_pm_enable();
  start_ota_rollback_watchdog(2, 120000);

  // ota_controller = new HAPPY::Entities::OtaController(screenie_device, "1.0.0");
  install_screenie_sensors();

  network.time_sync_callback = [](struct timeval* /*tv*/) {
    startup_gate_passed("NTP Time Synced");

    publish_screenie_sensors(true);

    network.time_sync_callback = [](struct timeval* /*tv*/) { publish_screenie_sensors(true); };
  };

  // Entities must be registered before the network is started so discovery messages are not missed.
  network.start();

  ESP_LOGI(TAG, "Starting main loop...");
  main_loop.run_forever();
}