#include "prometheus/exposer.h"
#include "prometheus/family.h"
#include "prometheus/gauge.h"
#include "prometheus/registry.h"
#include <iostream>
#include <thread>
#include <vector>
extern "C" {
#include <libsmu.h>
}

const int SMU_FETCH_INTERVAL_SECS = 5;

struct Metrics {
  prometheus::Gauge& cpu_power;
  prometheus::Gauge& soc_power;
  prometheus::Gauge& mem_power;
  prometheus::Gauge& vdd18_power;
  prometheus::Gauge& roc_power;
  prometheus::Gauge& socket_power;
};

int process_pm_table(const smu_obj_t& smu, float* buf, Metrics* metrics) {
  // Credit for these values goes to https://github.com/hattedsquirrel/ryzen_monitor/blob/main/src/pm_tables.c

  switch (smu.pm_table_version) {
  case 0x380805:
  case 0x240903:
    metrics->cpu_power.Set(buf[24]);
    metrics->soc_power.Set(buf[25]);
    metrics->mem_power.Set(buf[26]);
    metrics->vdd18_power.Set(buf[27]);
    metrics->roc_power.Set(buf[28]);
    metrics->socket_power.Set(buf[29]);

    break;
  default:
    std::cout << "PM table version " << std::hex << smu.pm_table_version << " is not supported.";
    return 1;
  }

  return 0;
}

int main() {
  auto addr = "0.0.0.0:9095";
  std::cout << "Using address" << addr << std::endl;
  prometheus::Exposer promExposer{addr};

  auto metricsRegistry = std::make_shared<prometheus::Registry>();
  auto& powerGauge = prometheus::BuildGauge().Name("smu_power").Help("Power usage").Register(*metricsRegistry);

  struct Metrics metrics = {
      powerGauge.Add({{"type", "CPU"}}),
      powerGauge.Add({{"type", "SOC"}}),
      powerGauge.Add({{"type", "MEM"}}),
      powerGauge.Add({{"type", "VDD18"}}),
      powerGauge.Add({{"type", "ROC"}}),
      powerGauge.Add({{"type", "Socket"}}),
  };

  promExposer.RegisterCollectable(metricsRegistry);

  smu_obj_t smu;
  auto result = smu_init(&smu);

  if (result != SMU_Return_OK) {
    std::cout << "SMU initialization failed: " << smu_return_to_str(result) << std::endl;
    return 1;
  }

  auto codename = smu_codename_to_str(&smu);
  auto fwVersion = smu_get_fw_version(&smu);

  std::cout << "Initialized SMU for " << codename << std::endl
            << "FW version: " << fwVersion << std::endl
            << "PM table version: " << std::hex << smu.pm_table_version << std::endl;

  if (smu_pm_tables_supported(&smu) != 1) {
    std::cout << "PM tables are not supported!" << std::endl;
    return 1;
  }

  for (;;) {
    std::vector pm_table_buf(smu.pm_table_size, (unsigned char)0);
    result = smu_read_pm_table(&smu, pm_table_buf.data(), pm_table_buf.size());

    if (result != SMU_Return_OK) {
      std::cout << "PM table error: " << smu_return_to_str(result) << std::endl;
      return 1;
    }

    auto code = process_pm_table(smu, (float*)(pm_table_buf.data()), &metrics);
    if (code != 0) {
      std::cout << "Error" << std::endl;
      return code;
    }

    std::this_thread::sleep_for(std::chrono::seconds(SMU_FETCH_INTERVAL_SECS));
  }

  return 0;
}
