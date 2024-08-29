#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>

#include "board_framework.hpp"
#include "board_framework_arduino.hpp"
#include "machine.hpp"

#define STR(x) #x
#define XSTR(x) STR(x)

const char* wifi_ssid = XSTR(WIFI_SSID);
const char* wifi_pass = XSTR(WIFI_PASS);
const char* mdns_addr = XSTR(MDNS_ADDR);
const char* ota_pass = XSTR(OTA_PASS);
const char* mqtt_broker = XSTR(MQTT_BROKER);
const uint16_t mqtt_port = MQTT_PORT;
const char* set_stock_topic = "smartfridge/set-stock";
const char* stock_topic = "smartfridge/current-stock";

BoardFramework* board;
machine_t *machine_state; 
int selected_product;

#define MACHINE_STATS_LEN 256
char machine_stats_buffer[MACHINE_STATS_LEN];

WiFiClient espClient;
PubSubClient client(espClient);

void publish_stock() {
    bool error = machine_state->to_json(machine_stats_buffer, MACHINE_STATS_LEN);
    if (not error) {
        board->log("Publishing stock statistics to 'smartfridge-stock'\n");
        client.publish(stock_topic, machine_stats_buffer);
    } else {
        board->log("[ERROR] Buffer overflow detected while serializing machine stock statistics.\n");
    }
}

// Callback function header
void callback(char* topic, byte* payload, unsigned int length) {
    board->log("Message arrived.\n");
    if (strcmp(set_stock_topic, topic) == 0) {
        /*
         * Waiting for a message to setup the stock of the machine.
         * eg: 
          {
            "stats": {
              "p0_stock": 8,
              "p1_stock": 8,
              "p2_stock": 8,
              "p3_stock": 8,
              "p4_stock": 8
            }
          }
         * */
        machine_state->set_product_stats_from_json((const char*) payload);
        publish_stock();
    }
}

void reconnect() {
    // Loop until we are connected
    while (!client.connected()) {
        board->log("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("mqtt-smartfridge")) {
            board->log("connected\n");
            // Subscribe
            snprintf(machine_stats_buffer,
                    MACHINE_STATS_LEN,
                    "smartfridge online, ip: %s", WiFi.localIP().toString().c_str());
            client.publish(stock_topic, machine_stats_buffer);
            client.subscribe(set_stock_topic);
            client.setCallback(callback);
        } else {
            board->log(" try again in 5 seconds\n");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    board = new BoardFrameworkArduino();
    machine_state = new Machine(board);

    Serial.begin(115200);

    // Connect ESP32 to the defined access point
    WiFi.mode(WIFI_STA); 
    WiFi.begin(wifi_ssid, wifi_pass);

    board->log("\nConnecting...");
    while(WiFi.status() != WL_CONNECTED) {
        board->log(".");
        delay(100);
    }
    board->log("\nConnected to the WiFi network");
    board->log("Local ESP32 IP: ");
    board->log(WiFi.localIP().toString().c_str());

    // Setup MDNS
    if (!MDNS.begin(mdns_addr)) {
        board->log("Error starting mDNS");
        return;
    }
    board->log("mDNS responder started");

    // Setup connection to MQTT broker.
    client.setServer(mqtt_broker, mqtt_port);
}


void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    machine_state->read_buttons();
    
    selected_product = machine_state->deliver_product();
    if (selected_product != -1) {
        publish_stock();
    }
}
