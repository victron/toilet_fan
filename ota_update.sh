#!/bin/bash
# -------------- install arduino-cli
# curl -fsSL https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz -o arduino-cli.tar.gz
# tar -xzf arduino-cli.tar.gz
# sudo mv arduino-cli /usr/local/bin/
# arduino-cli version

# --------------- config init
# arduino-cli config init
# vi /home/vic/.arduino15/arduino-cli.yaml
# # vic@HA2:~/ota_update$ cat /home/vic/.arduino15/arduino-cli.yaml
# # board_manager:
# #   additional_urls:
# #     - http://arduino.esp8266.com/stable/package_esp8266com_index.json
# arduino-cli core update-index
# arduino-cli core search esp8266
# arduino-cli core install esp8266:esp8266

# ------------------ clone and update repo with code
# git clone git@github.com:victron/bath_fan.git
# add secrets.h
# add secrets.sh


# ------------------- install custom libs
# arduino-cli core install home-assistant-integration
# arduino-cli lib install "Adafruit BME280 Library"
project_name="${PWD##*/}"
. ./secrets.sh
espota='/home/vic/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/tools/espota.py'
build_cache='~/ota_update/build_cache'
builds_dir='~/ota_update/build'

git pull
git log -1
echo "------- start combile ---------"
echo $builds_dir
echo ${project_name}
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-cache-path ${build_cache} --output-dir ${builds_dir} ./${project_name}.ino
if [ $? -ne 0 ]; then
  echo "Команда завершилась з помилкою. Завершення скрипта."
  exit 1
fi
echo ${builds_dir}/${project_name}
python3 ${espota} --ip ${DEVICE_IP} --auth=${OTA_PASSWORD} --file ${builds_dir}/${project_name}.ino.bin


