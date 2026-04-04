# CLAUDE.md — ESP-NOW Remote Control Project

## Project Overview

A wireless 3-button remote control system using ESP-NOW protocol.

- **Remote (TX):** Seeed XIAO ESP32-C3 — battery-powered, 3 buttons, deep sleep between presses
- **Base Station (RX):** Lolin S2 Mini (ESP32-S2) — USB-connected, receives ESP-NOW commands and presents as USB HID device to the host computer

The remote sends button press events over ESP-NOW. The base station receives them and translates to HID keypresses (or media keys, etc.) over USB.

## Tech Stack

- **Framework:** Arduino (via PlatformIO)
- **Build System:** PlatformIO
- **Language:** C/C++ (Arduino-flavored)
- **Protocol:** ESP-NOW (connectionless, peer-to-peer, 250 byte max payload)
- **IDE:** VS Code with PlatformIO extension
- **CAD:** Autodesk Fusion 360 (.f3d files in /mechanical — binary, not editable by Claude)

## Repository Structure

```
esp-now-remote/
├── CLAUDE.md              ← you are here
├── README.md
├── .gitignore
├── firmware/
│   ├── platformio.ini     ← multi-environment config (remote + base_station)
│   ├── src/
│   │   ├── remote/
│   │   │   └── main.cpp   ← TX firmware entry point
│   │   └── base_station/
│   │       └── main.cpp   ← RX firmware entry point
│   ├── lib/
│   │   └── shared/        ← shared library: protocol defs, message structs, config
│   │       ├── shared.h
│   │       └── shared.cpp
│   └── include/            ← (currently unused, for future global headers)
├── mechanical/             ← Fusion 360 .f3d files (binary, do not edit)
└── docs/                   ← wiring diagrams, protocol notes, etc.
```

## Build & Flash Commands

All commands run from the `firmware/` directory.

```bash
# Build remote firmware
pio run -e remote

# Build base station firmware
pio run -e base_station

# Build both
pio run

# Upload to remote (connect XIAO ESP32-C3 via USB)
pio run -e remote -t upload

# Upload to base station (connect Lolin S2 Mini via USB)
pio run -e base_station -t upload

# Serial monitor (specify port if needed)
pio device monitor -e remote
pio device monitor -e base_station
```

## PlatformIO Environment Names

- `remote` — targets the Seeed XIAO ESP32-C3
- `base_station` — targets the Lolin S2 Mini (ESP32-S2)

Each environment has its own `src_filter` in platformio.ini so only the relevant source files are compiled per target.

## Shared Library (lib/shared/)

The `shared` library contains everything both targets must agree on:
- `message_t` struct — the ESP-NOW payload format
- Button command enum/defines
- ESP-NOW channel number
- Any MAC address configuration

When modifying the protocol, always update `shared.h` — both targets include it. Changing the message struct on one side without the other will cause silent failures.

## Hardware Notes

### Remote (XIAO ESP32-C3)
- 3 GPIO pins for buttons (active LOW with internal pullup)
- Built-in LiPo battery management (charge via USB-C)
- Deep sleep between button presses to conserve battery
- Pin assignments: defined in `src/remote/main.cpp` (see TODO for final pin selection)

### Base Station (Lolin S2 Mini)
- ESP32-S2 has native USB (USB-OTG), no UART-to-USB bridge
- Acts as USB HID device to the host computer
- Always powered via USB from the host
- Uses the TinyUSB stack (included in Arduino-ESP32 for S2)

## Coding Conventions

- Use `#define` or `constexpr` for pin assignments at the top of each main.cpp
- Prefer `Serial.println()` debug output guarded by a `#define DEBUG` flag
- Use `snake_case` for functions and variables
- Use `UPPER_SNAKE_CASE` for constants and defines
- Keep functions short — extract helpers rather than writing 100-line `loop()` bodies
- Comment *why*, not *what* — assume the reader knows C++ and Arduino APIs

## Common Pitfalls

- **ESP-NOW channel:** Both devices MUST be on the same Wi-Fi channel. Default is channel 1. If one side inits Wi-Fi in STA mode, it may auto-switch channels. Explicitly set the channel after WiFi.mode().
- **MAC addresses:** ESP-NOW requires the receiver's MAC address for unicast. For broadcast, use `FF:FF:FF:FF:FF:FF`. Broadcast is fine for a single-remote-single-base setup.
- **S2 USB HID:** The Lolin S2 Mini uses native USB. When flashing, you may need to hold BOOT + tap RST to enter download mode. After flashing HID firmware, the board will enumerate as an HID device, not a serial port — use a `#define DEBUG` to conditionally enable CDC serial alongside HID for debugging.
- **Deep sleep wakeup (C3):** Use `esp_deep_sleep_enable_gpio_wakeup()` on the button pins. After wakeup, the chip reboots — `setup()` runs again. Design accordingly.

## What Claude Should NOT Do

- Do not modify .f3d files (binary CAD files)
- Do not change the PlatformIO board identifiers without confirming with the user
- Do not add Arduino library dependencies without discussing — prefer ESP-IDF APIs available through the Arduino-ESP32 core where possible
- Do not create separate PlatformIO projects — this is a single-project, multi-environment setup
