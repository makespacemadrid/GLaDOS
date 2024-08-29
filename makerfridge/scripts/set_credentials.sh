#/bin/env bash
#set -x

# The purpose of this script is to set the credentials in platformio.ini
# AFTER submitting changes to version control.

# Get the Git root directory
git_root=$(git rev-parse --show-toplevel 2>/dev/null)

export INI_FILE=$git_root/makerfridge/platformio.ini

export FIELD_WIFI_SSID='wifi_ssid ='
export FIELD_WIFI_PASS='wifi_pass ='
export FIELD_MDNS_ADDR='mdns_addr ='
export FIELD_OTA_PASS='ota_pass ='
export FIELD_MQTT_BROKER='mqtt_broker ='
export FIELD_MQTT_PORT='mqtt_port ='

if [[ -z "$WIFI_SSID" ]]; then
    echo "WIFI_SSID not set, check up your environment variables."
    exit
fi
if [[ -z "$WIFI_PASS" ]]; then
    echo "WIFI_PASS not set, check up your environment variables."
    exit
fi
if [[ -z "$MDNS_ADDR" ]]; then
    echo "MDNS_ADDR not set, check up your environment variables."
    exit
fi
if [[ -z "$OTA_PASS" ]]; then
    echo "OTA_PASS not set, check up your environment variables."
    exit
fi
if [[ -z "$MQTT_BROKER" ]]; then
    echo "MQTT_BROKER not set, check up your environment variables."
    exit
fi
if [[ -z "$MQTT_PORT" ]]; then
    echo "MQTT_PORT not set, check up your environment variables."
    exit
fi

sed -i "/$FIELD_WIFI_SSID/c$FIELD_WIFI_SSID '$WIFI_SSID'" $INI_FILE
sed -i "/$FIELD_WIFI_PASS/c$FIELD_WIFI_PASS '$WIFI_PASS'" $INI_FILE
sed -i "/$FIELD_MDNS_ADDR/c$FIELD_MDNS_ADDR '$MDNS_ADDR'" $INI_FILE
sed -i "/$FIELD_OTA_PASS/c$FIELD_OTA_PASS '$OTA_PASS'" $INI_FILE
sed -i "/$FIELD_MQTT_BROKER/c$FIELD_MQTT_BROKER '$MQTT_BROKER'" $INI_FILE
sed -i "/$FIELD_MQTT_PORT/c$FIELD_MQTT_PORT '$MQTT_PORT'" $INI_FILE


