#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

// ---------------------------------------------------------------------------
// ESP-NOW Configuration
// ---------------------------------------------------------------------------

// Wi-Fi channel — both devices MUST use the same channel
#define ESPNOW_CHANNEL 1

// Broadcast address — fine for single-remote, single-base topology
static const uint8_t BROADCAST_ADDR[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------------------------------------------------------------------
// Button Command Definitions
// ---------------------------------------------------------------------------

typedef enum : uint8_t {
    CMD_NONE      = 0x00,
    CMD_BUTTON_1  = 0x01,
    CMD_BUTTON_2  = 0x02,
    CMD_BUTTON_3  = 0x03,
    CMD_BUTTON_4  = 0x04,
} button_cmd_t;

// ---------------------------------------------------------------------------
// ESP-NOW Message Payload
// ---------------------------------------------------------------------------

// Keep this struct packed and under 250 bytes (ESP-NOW max payload)
typedef struct __attribute__((packed)) {
    uint8_t       version;    // protocol version — increment on breaking changes
    button_cmd_t  command;    // which button was pressed
    uint8_t       reserved;   // future use (long press, battery level, etc.)
} message_t;

#define PROTOCOL_VERSION 1

#endif // SHARED_H
