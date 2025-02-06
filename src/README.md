# Instructions to install platformIO env on HA

## prepare env
```
python3 -m venv platformIO
. platformIO/bin/activate
pip3 install platformio
pio --version # check

# set github PAT
. secrets.sh
git clone https://${PAT}@github.com/victron/Room_fan.git
cd Room_fan
vi src/secrets.h
vi src/secrets.sh

pio pkg install
pio pkg list


pio run -t clean
pio run
```

## ota
### get ip
```
sudo cat /var/lib/NetworkManager/dnsmasq-wlp3s0.leases
```

### edit platformio.ini
```
upload_protocol = espota
upload_port = ${sysenv.DEVICE_IP}
upload_flags =
    --port=3232
    --auth=${sysenv.OTA_PASSWORD}
```

### OTA update
```
. ~/platformIO/bin/activate
cd ~/ota_update_pio/Room_fan/
. src/secrets.sh

pio run -e ota --target upload
```