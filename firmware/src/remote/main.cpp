// ==========================================================================
// Remote (TX) — Seeed XIAO ESP32-C3
// 4-button ESP-NOW transmitter with deep sleep
// ==========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include "shared.h"

// ---------------------------------------------------------------------------
// Pin Definitions — update these to match your wiring
// ---------------------------------------------------------------------------
#define BTN_1_PIN  3   // D1 on XIAO ESP32-C3 silkscreen
#define BTN_2_PIN  4   // D2 on XIAO ESP32-C3 silkscreen
#define BTN_3_PIN  5   // D3 on XIAO ESP32-C3 silkscreen
#define BTN_4_PIN  2   // D0 on XIAO ESP32-C3 silkscreen (strapping pin — safe at runtime)

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static volatile bool send_complete = false;
static volatile bool send_success  = false;

// ---------------------------------------------------------------------------
// ESP-NOW send callback
// ---------------------------------------------------------------------------
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    send_success  = (status == ESP_NOW_SEND_SUCCESS);
    send_complete = true;
}

// ---------------------------------------------------------------------------
// Determine which button triggered wakeup (or was pressed at boot)
// ---------------------------------------------------------------------------
button_cmd_t read_button() {
    if (digitalRead(BTN_1_PIN) == LOW) return CMD_BUTTON_1;
    if (digitalRead(BTN_2_PIN) == LOW) return CMD_BUTTON_2;
    if (digitalRead(BTN_3_PIN) == LOW) return CMD_BUTTON_3;
    if (digitalRead(BTN_4_PIN) == LOW) return CMD_BUTTON_4;
    return CMD_NONE;
}

// ---------------------------------------------------------------------------
// Enter deep sleep with GPIO wakeup on any button press
// ---------------------------------------------------------------------------
void enter_deep_sleep() {
    // Configure wakeup on any button pin going LOW
    // Note: esp_deep_sleep_enable_gpio_wakeup() uses a bitmask of GPIO numbers
    uint64_t wakeup_mask = (1ULL << BTN_1_PIN) | (1ULL << BTN_2_PIN) | (1ULL << BTN_3_PIN) | (1ULL << BTN_4_PIN);
    esp_deep_sleep_enable_gpio_wakeup(wakeup_mask, ESP_GPIO_WAKEUP_GPIO_LOW);

    #ifdef DEBUG
    Serial.println("Entering deep sleep...");
    Serial.flush();
    #endif

    esp_deep_sleep_start();
}

// ---------------------------------------------------------------------------
// Setup — runs on every boot (including wakeup from deep sleep)
// ---------------------------------------------------------------------------
void setup() {
    #ifdef DEBUG
    Serial.begin(115200);
    delay(100);
    Serial.println("Remote booting...");
    #endif

    // Configure button pins
    pinMode(BTN_1_PIN, INPUT_PULLUP);
    pinMode(BTN_2_PIN, INPUT_PULLUP);
    pinMode(BTN_3_PIN, INPUT_PULLUP);
    pinMode(BTN_4_PIN, INPUT_PULLUP);

    // Small debounce delay
    delay(50);

    // Read which button is pressed
    button_cmd_t cmd = read_button();
    if (cmd == CMD_NONE) {
        // Spurious wakeup or no button pressed — go back to sleep
        #ifdef DEBUG
        Serial.println("No button detected, sleeping.");
        #endif
        enter_deep_sleep();
        return;
    }

    #ifdef DEBUG
    Serial.printf("Button %d pressed\n", cmd);
    #endif

    // Init Wi-Fi in STA mode (required for ESP-NOW)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        #ifdef DEBUG
        Serial.println("ESP-NOW init failed");
        #endif
        enter_deep_sleep();
        return;
    }

    esp_now_register_send_cb(on_data_sent);

    // Register broadcast peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, BROADCAST_ADDR, 6);
    peer.channel = ESPNOW_CHANNEL;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    // Build and send message
    message_t msg = {};
    msg.version  = PROTOCOL_VERSION;
    msg.command  = cmd;
    msg.reserved = 0;

    esp_now_send(BROADCAST_ADDR, (uint8_t *)&msg, sizeof(msg));

    // Wait for send callback (with timeout)
    unsigned long start = millis();
    while (!send_complete && (millis() - start < 500)) {
        delay(1);
    }

    #ifdef DEBUG
    if (send_complete && send_success) {
        Serial.println("Send OK");
    } else {
        Serial.println("Send FAILED or timed out");
    }
    #endif

    // Done — go to sleep
    esp_now_deinit();
    enter_deep_sleep();
}

void loop() {
    // Never reached — deep sleep reboots into setup()
}
