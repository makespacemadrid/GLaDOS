#/bin/env bash

# The purpose of this script is to removing the credentials in platformio.ini
# BEFORE submitting changes to version control.

# Get the Git root directory
git_root=$(git rev-parse --show-toplevel 2>/dev/null)

export INI_FILE=$git_root/makerfridge/platformio.ini

export FIELD_WIFI_SSID='wifi_ssid ='
export FIELD_WIFI_PASS='wifi_pass ='
export FIELD_MDNS_ADDR='mdns_addr ='
export FIELD_OTA_PASS='ota_pass ='
export FIELD_MQTT_BROKER='mqtt_broker ='
export FIELD_MQTT_PORT='mqtt_port ='

export WIFI_SSID='yourssid'
export WIFI_PASS='yourpassword'
export MDNS_ADDR='smartfridge'
export OTA_PASS='yourotapass'
export MQTT_BROKER='yourmqttbroker'
export MQTT_PORT=1234

sed -i "/$FIELD_WIFI_SSID/c$FIELD_WIFI_SSID '$WIFI_SSID'" $INI_FILE
sed -i "/$FIELD_WIFI_PASS/c$FIELD_WIFI_PASS '$WIFI_PASS'" $INI_FILE
sed -i "/$FIELD_MDNS_ADDR/c$FIELD_MDNS_ADDR '$MDNS_ADDR'" $INI_FILE
sed -i "/$FIELD_OTA_PASS/c$FIELD_OTA_PASS '$OTA_PASS'" $INI_FILE
sed -i "/$FIELD_MQTT_BROKER/c$FIELD_MQTT_BROKER '$MQTT_BROKER'" $INI_FILE
sed -i "/$FIELD_MQTT_PORT/c$FIELD_MQTT_PORT $MQTT_PORT" $INI_FILE
