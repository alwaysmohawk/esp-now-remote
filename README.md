# ESP-NOW Remote Control

A wireless 3-button remote control using ESP-NOW protocol.

## Hardware

| Role | Board | Chip | Notes |
|------|-------|------|-------|
| Remote (TX) | Seeed XIAO ESP32-C3 | ESP32-C3 | Battery powered, deep sleep between presses |
| Base Station (RX) | Lolin S2 Mini | ESP32-S2 | USB-connected, acts as HID device |

## How It Works

1. Press a button on the remote → wakes from deep sleep
2. Remote sends a command byte over ESP-NOW (broadcast)
3. Base station receives the command
4. Base station sends a USB HID keypress to the host computer
5. Remote goes back to deep sleep

## Building

Requires [PlatformIO](https://platformio.org/). From the `firmware/` directory:

```bash
# Build both targets
pio run

# Build and upload remote only
pio run -e remote -t upload

# Build and upload base station only
pio run -e base_station -t upload

# Serial monitor
pio device monitor -e remote
```

## Project Structure

```
firmware/          PlatformIO project (multi-environment)
  src/remote/      TX firmware (XIAO ESP32-C3)
  src/base_station/ RX firmware (Lolin S2 Mini)
  lib/shared/      Common protocol definitions
mechanical/        Fusion 360 CAD files for enclosures
docs/              Wiring diagrams, notes
```

## Status

working well, pins labelled D1/D2/D3 (which are actually GPIO3/GPIO4/GPIO5) cause the receiver to type a/s/d.
