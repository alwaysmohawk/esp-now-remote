// ==========================================================================
// Base Station Serial (RX) — NodeMCU ESP-32S
// ESP-NOW receiver → Serial output
// ==========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "shared.h"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static volatile bool      msg_received = false;
static volatile message_t last_msg     = {};

// ---------------------------------------------------------------------------
// ESP-NOW receive callback
// ---------------------------------------------------------------------------
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (len != sizeof(message_t)) {
        Serial.printf("Bad payload size: %d (expected %d)\n", len, sizeof(message_t));
        return;
    }

    memcpy((void *)&last_msg, data, sizeof(message_t));

    if (last_msg.version != PROTOCOL_VERSION) {
        Serial.printf("Version mismatch: got %d, expected %d\n", last_msg.version, PROTOCOL_VERSION);
        return;
    }

    msg_received = true;
}

// ---------------------------------------------------------------------------
// Handle a received command — print to serial
// ---------------------------------------------------------------------------
void handle_command(button_cmd_t cmd) {
    switch (cmd) {
        case CMD_BUTTON_1: Serial.println("a"); break;
        case CMD_BUTTON_2: Serial.println("b"); break;
        case CMD_BUTTON_3: Serial.println("c"); break;
        case CMD_BUTTON_4: Serial.println("d"); break;
        default:
            Serial.printf("Unknown command: 0x%02X\n", cmd);
            break;
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("Base station (serial) booting...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed!");
        return;
    }

    esp_now_register_recv_cb(on_data_recv);

    Serial.println("Ready. Listening for ESP-NOW...");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
}

// ---------------------------------------------------------------------------
// Main loop — process received messages
// ---------------------------------------------------------------------------
void loop() {
    if (msg_received) {
        msg_received = false;
        handle_command(last_msg.command);
    }

    delay(1); // yield to background tasks
}
