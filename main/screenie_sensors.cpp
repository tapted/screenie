#include "screenie_sensors.hpp"

#include "espbase/main_loop_task.hpp"
#include "halpp/config.hpp"
#include "happy/entities/system_diagnostics.hpp"
#include "screenie_device.hpp"
#include "screenie_hardware.hpp"

using halpp::config;

static HAPPY::Entities::SystemDiagnostics* diagnostics = nullptr;

static constinit MainLoopTask<void> publish_sensors_on_time_interval;

void install_screenie_sensors() {
  init_screenie_hardware();

  diagnostics = new HAPPY::Entities::SystemDiagnostics(screenie_device);

  publish_sensors_on_time_interval.start({.name = "publish_sensors"}, nullptr,
                                         [](auto&) -> std::optional<uint32_t> {
                                           publish_screenie_sensors();
                                           return 60000;  // Re-run every 60 seconds
                                         });
  screenie_device.load();
}

void publish_screenie_sensors(bool time_sync) {
  diagnostics->publish_all_mutable(time_sync);
}