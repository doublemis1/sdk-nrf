
#!/bin/bash

set -e
cd $(dirname "$0")

for suffix in "_1" "_2"; do
  rm -rf build
  west build -b nrf52840dk_nrf52840 -t factory_data_hex -- -DOVERLAY_CONFIG="overlay-factory_data_build${suffix}.conf"

  mkdir -p factory_data_new${suffix}
  cp build/zephyr/{factory_data}* factory_data_new${suffix}
done
