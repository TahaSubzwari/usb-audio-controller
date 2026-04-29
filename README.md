# USB Audio Controller

 

 A plug-and-play hardware volume controller built on the STM32F401RE (ARM Cortex-M4).

 Turn a rotary encoder knob to control system volume, press it to mute.

 A 2-inch SPI LCD displays the current volume level in real time.

 No drivers required — the device enumerates as a standard HID Consumer Control device on Windows.

 

## Features

 

- Rotary encoder input

- Press encoder to mute/unmute with visual indicator on LCD

- USB HID Consumer Control — recognized natively by Windows, no drivers needed

- Arc-style volume dial on 2-inch SPI LCD with partial redraw

- LCD synced to actual PC volume via UART — stays accurate regardless of what changed the volume (knob, keyboard, Windows mixer)

- Companion tray app (Windows) runs silently in the background, optional run-on-startup via system tray menu

 

## Hardware

| Component | Details |
|---|---|
| Microcontroller | STM32 Nucleo F401RE (ARM Cortex-M4 @ 84MHz) |
| Display | Waveshare 2-inch SPI LCD (240×240) |
| Input | 5-pin rotary encoder (CLK → PB15, DT → PB14, SW → PB13) |
| USB HID | Micro-USB breakout wired to PA11 (D−) and PA12 (D+) |
| PC Sync | ST-Link UART (USART2) used for volume feedback from companion app |

 

## Peripherals Used

 

| Peripheral | Purpose |
|---|---|
| SPI1 | LCD communication |
| TIM3 | LCD backlight PWM |
| USB OTG FS | HID Consumer Control (PA11/PA12) |
| USART2 | Volume sync from companion app |
| GPIO PB13 | Encoder switch (mute) |
| GPIO PB14 | Encoder DT |
| GPIO PB15 | Encoder CLK |

 

## Companion App

 

 The Windows tray app (`USBAudioController.py`) keeps the LCD synchronized

 with actual PC volume.

 

## Roadmap

 

- \[x] SPI LCD with custom volume dial UI

- \[x] Rotary encoder input with debounce

- \[x] USB HID Consumer Control — volume and mute

- \[x] PC volume sync via UART companion app

- \[x] Windows tray app with startup option

- \[ ] KiCAD schematic

- \[ ] Custom PCB

 

## Development Environment

 

- STM32CubeIDE 1.2

- STM32 HAL (CubeMX generated)

- Waveshare LCD library (adapted for STM32F401RE)

