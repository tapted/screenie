#include "screenie_sensors.hpp"

#include <esp_heap_trace.h>
#include <esp_log.h>

#include "espbase/main_loop_task.hpp"
#include "halpp/config.hpp"
#include "happy/entities/ota.hpp"
#include "happy/entities/switch.hpp"
#include "happy/entities/system_diagnostics.hpp"
#include "screenie_device.hpp"
#include "screenie_hardware.hpp"

using halpp::config;

static void heap_trace(bool enable) {
#if 0
  static constexpr size_t NUM_TRACE_RECORDS = 50;
  static bool is_initialized = false;
  static heap_trace_record_t trace_records[NUM_TRACE_RECORDS];

  if (!enable && !is_initialized) return;

  // 1. Lazy Initialization (Only runs on the very first click)
  if (!is_initialized) {
    esp_err_t err = heap_trace_init_standalone(trace_records, NUM_TRACE_RECORDS);
    if (err != ESP_OK) {
      ESP_LOGE("HeapTrace", "Failed to init heap trace");
      return;
    }
    is_initialized = true;
  }

  // 2. Toggle Logic
  if (enable) {
    ESP_LOGI("HeapTrace", "Starting heap trace...");
    heap_trace_start(HEAP_TRACE_ALL);
  } else {
    ESP_LOGI("HeapTrace", "Stopping and dumping heap trace...");
    heap_trace_stop();
    heap_trace_dump();
  }
#else
  ESP_LOGW("HeapTrace", "Heap tracing is disabled in this build.");
#endif
}

static HAPPY::Entities::SystemDiagnostics* diagnostics = nullptr;
static HAPPY::Entities::OtaController* ota_controller = nullptr;

static HAPPY::Entities::Switch heap_dump_switch(
    screenie_device, "heap_dump", "Heap Dump",
    {
        .icon = "mdi:memory",
        .entity_category = "config",
        .on_change = [](void*, const HAPPY::Entities::Switch& s) { heap_trace(s.is_on()); },
    });

static constinit MainLoopTask<void> publish_sensors_on_time_interval;

void install_screenie_sensors() {
  init_screenie_hardware();

  ota_controller = new HAPPY::Entities::OtaController(screenie_device, "1.0.0");
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