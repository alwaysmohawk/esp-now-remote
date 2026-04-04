// ==========================================================================
// Base Station (RX) — Lolin S2 Mini (ESP32-S2)
// ESP-NOW receiver → USB HID output
// ==========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "shared.h"

// ---------------------------------------------------------------------------
// USB HID (TinyUSB) — ESP32-S2 native USB
// ---------------------------------------------------------------------------
// TODO: Add TinyUSB / USB HID includes and descriptor setup
// The Arduino-ESP32 core for S2 includes TinyUSB support.
// Uncomment and configure once the basic ESP-NOW RX is working.
//
// #include "USB.h"
// #include "USBHIDKeyboard.h"
// USBHIDKeyboard Keyboard;

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static volatile bool     msg_received = false;
static volatile message_t last_msg    = {};

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

    switch (cmd) {
        case CMD_BUTTON_1:
            // TODO: send HID keypress for button 1
            // Keyboard.press(KEY_F13);
            // delay(50);
            // Keyboard.release(KEY_F13);
            #ifdef DEBUG
            Serial.println("BTN1 → HID key TBD");
            #endif
            break;

        case CMD_BUTTON_2:
            // TODO: send HID keypress for button 2
            #ifdef DEBUG
            Serial.println("BTN2 → HID key TBD");
            #endif
            break;

        case CMD_BUTTON_3:
            // TODO: send HID keypress for button 3
            #ifdef DEBUG
            Serial.println("BTN3 → HID key TBD");
            #endif
            break;

        default:
            #ifdef DEBUG
            Serial.println("Unknown command");
            #endif
            break;
    }
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

    // TODO: Initialize USB HID
    // Keyboard.begin();
    // USB.begin();

    // Init Wi-Fi in STA mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // TODO: explicitly set Wi-Fi channel
    // esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

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
