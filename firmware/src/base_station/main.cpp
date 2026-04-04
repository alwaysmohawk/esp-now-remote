// ==========================================================================
// Base Station (RX) — Lolin S2 Mini (ESP32-S2)
// ESP-NOW receiver → USB HID output
// ==========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "shared.h"

// ---------------------------------------------------------------------------
// USB HID
// ---------------------------------------------------------------------------
USBHIDKeyboard Keyboard;

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
        #ifdef DEBUG
        Serial.printf("Bad payload size: %d (expected %d)\n", len, sizeof(message_t));
        #endif
        return;
    }

    memcpy((void *)&last_msg, data, sizeof(message_t));

    if (last_msg.version != PROTOCOL_VERSION) {
        #ifdef DEBUG
        Serial.printf("Version mismatch: got %d, expected %d\n", last_msg.version, PROTOCOL_VERSION);
        #endif
        return;
    }

    msg_received = true;
}

// ---------------------------------------------------------------------------
// Handle a received command — translate to HID keypress
// ---------------------------------------------------------------------------
void handle_command(button_cmd_t cmd) {
    #ifdef DEBUG
    Serial.printf("Handling command: 0x%02X\n", cmd);
    #endif

    uint8_t key = 0;
    switch (cmd) {
        case CMD_BUTTON_1: key = 'a'; break;
        case CMD_BUTTON_2: key = 's'; break;
        case CMD_BUTTON_3: key = 'd'; break;
        default:
            #ifdef DEBUG
            Serial.println("Unknown command");
            #endif
            return;
    }

    Keyboard.press(key);
    delay(50);
    Keyboard.release(key);
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    #ifdef DEBUG
    Serial.begin(115200);
    delay(100);
    Serial.println("Base station booting...");
    #endif

    Keyboard.begin();
    #ifdef DEBUG
    USB.enableDFU(); // allows reflashing without boot+reset button dance
    #endif
    USB.begin();

    // Init Wi-Fi in STA mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        #ifdef DEBUG
        Serial.println("ESP-NOW init failed!");
        #endif
        return;
    }

    esp_now_register_recv_cb(on_data_recv);

    #ifdef DEBUG
    Serial.println("Base station ready. Listening for ESP-NOW...");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    #endif
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
