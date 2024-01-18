#ifndef STRINGS_H
#define STRINGS_H

#define JSONCONFIGSIZE  1000

//Campos JSON
#define NODE_ID      "id"
#define NODE_TYPE    "type"

#define WIFI_MODE    F("wmode")
#define WIFI_ESSID   F("essid")
#define WIFI_PWD     F("wpwd")

#define STATIC_IP_EN F("sipen")
#define STATIC_IP    F("sip")

#define MQTT_ENABLED F("mqEn")
#define MQTT_SERVER  F("mqHost")
#define MQTT_PORT    F("mqPort")

#define STATUS_PIN   "spin"
#define CONFIG_PIN   "cpin"
#define CONFIG_MODE  "configM"
#define MAX_BERROR   "maxBErr"
#define BERROR       "berr"

#define LOOP_DELAY     F("lpdelay")
#define SENSOR_REFRESH F("sdelay")
#define LED_REFRESH    F("ldelay")
#define NODE_REFRESH   F("ndelay")
#define OTHER_REFRESH  F("odelay")
#define WD_REFRESH     F("wdelay")
#define TELE_REFRESH   F("teleT")

#define ARTNET_ENABLED  F("artEn")
#define ARTNET_UNIVERSE F("artUn")
#define ARTNET_CHANNEL  F("artC")
#define ARTNET_ANNOUNCE F("artA")

#define SACN_ENABLED    F("sacnEn")
#define SACN_UNIVERSE   F("sacnUn")
#define SACN_CHANNEL    F("sacnC")

#define LED_HW              F("ledhw")
#define LED_COUNT           F("ledC")
#define LED_BRIGHT          F("ledB")
#define LED_MAXBRIGHT       F("ledMB")
#define UNDERVOLT_PROTECT   F("uProtect")
#define POWER_CALIBRATION   F("powerC")
#define FIRST_RUN           "fr"
#define NODE_CONFIG_VERSION F("nconfigv")
#define LEDS_CONFIG_VERSION F("lconfigv")

#define NODE_CONFIG         F("/nodeConfig.json")
#define LEDS_CONFIG         F("/ledController.json")
#define BOOT_CONFIG         "/boot.json"

//NODE NAMES
#define NODE_NEW         "new"
#define NODE_LEDBAR      "ledBar"
#define NODE_LEDMATRIX   "ledMatrix"
#define NODE_LEDKEY      "ledKey"
#define NODE_LEDTREE     "ledTree"
#define NODE_LEDREJIBAND "ledRejiband"
#define NODE_COFFEEMAKER "coffeeMaker"
//LED TYPES
#define LED_WS2812      F("ws2812")
#define LED_WS2801      F("ws2801")
#define LED_APA102      F("apa102")
// EFFECTS
#define E_FADE          F("fade")
#define E_FADETOCOLOR   F("fadeC")
#define E_SPARKS        F("sparks")
#define E_CYLON         F("cylon")
#define E_RAINBOW       F("rainbow")
#define E_CLIGHT        F("clight")
#define E_GLOW          F("glow")
#define E_COLOR         F("color")
#define E_RANDOM        F("random")
#define E_RANDOMCOLOR   F("randomColor")
#define E_OFF           F("off")
#define E_PROGRESS      F("progress")
#define E_FLASH         F("flash")
#define E_STROBE        F("strobe")
#define E_FIRE          F("fire")
#define E_GAME          F("game")
#define E_CHRISTMAS     F("christmas")
//Strings
#define ENABLED         "en"
#define DISABLED        "dis"
#define ON_BOOT_ERROR   "onB"
#define ON_PIN          "onP"
#define WIFI_CLIENT     F("wclient")
#define WIFI_MASTER     F("wmaster")

#endif
