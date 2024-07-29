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
OTA_UPLOAD_PORT=${DEVICE_IP}
OTA_AUTH=${OTA_PASSWORD}

git pull
git log -1
echo "------- start combile ---------"
pio run
if [ $? -ne 0 ]; then
  echo "Команда завершилась з помилкою. Завершення скрипта."
  exit 1
fi
pio run --target upload --upload-port <YOUR_DEVICE_IP>


 