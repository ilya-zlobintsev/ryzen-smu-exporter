# ryzen-smu-exporter

Prometheus exporter for values exposed by the [ryzen_smu](https://gitlab.com/leogx9r/ryzen_smu) driver.

# Building

```
git submodule update --init --recursive
meson setup build
cd build
meson compile
```
The output binary will be in `./build/ryzen_smu_exporter`

Exposed metrics:
```
# HELP smu_power Power usage
# TYPE smu_power gauge
smu_power{type="ROC"} 1
smu_power{type="VDD18"} 0.9253675937652588
smu_power{type="Socket"} 46.96342468261719
smu_power{type="MEM"} 10.03174495697021
smu_power{type="SOC"} 16.19046974182129
smu_power{type="CPU"} 18.77905654907227
```
