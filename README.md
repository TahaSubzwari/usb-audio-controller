# USB Audio Controller

A hardware volume controller built on the STM32F401RE (ARM Cortex-M4).
Turn a physical rotary encoder knob to control system volume in real time,
with live feedback on a 2-inch SPI LCD display. Driverless on Windows as a 
standard HID Consumer Control device.

## Features

- Rotary encoder input
- USB HID Consumer Control — recognized by Windows as a native media device
- Arc-style volume dial on 2-inch SPI LCD with partial redraw

## Hardware

| Component | Details |
|---|---|
| Microcontroller | STM32 Nucleo F401RE (ARM Cortex-M4 @ 84MHz) |
| Display | Waveshare 2-inch SPI LCD (240×240) |
| Input | 5-pin rotary encoder (CLK → PB15, DT → PB14, SW → PB13) |
| USB | Micro-USB breakout wired to PA11 (D−) and PA12 (D+) |

## How It Works

### Encoder Reading
CLK and DT are polled continuously in the main loop. The encoder is decoded using a quadrature 
polling approach. Direction is determined from the phase relationship between CLK and DT, 
and a full detent is only registered once both signals return to their idle HIGH state. 
A short debounce interval with full-cycle detection prevents false triggers from contact bounce.

### USB HID
The device is recognized as a HID Consumer Control device using a custom report
descriptor. Each detent sends a 2-byte press report (Report ID + Volume
Increment or Decrement bit), followed immediately by a release report. 
The release report must be sent within a single USB polling interval (~10 ms).
Delayed release reports can cause the host to interpret a single detent as multiple presses.

### LCD Display
The Waveshare library was adapted from the manufacturer's demo code, which
required changes to clock configuration and pin mapping to work on the F401RE. 
To avoid blocking the main loop, only the dynamic portions of the UI (arc and numeric value) 
are redrawn instead of refreshing the entire screen.

## Peripherals Used

| Peripheral | Purpose |
|---|---|
| SPI1 | LCD communication |
| TIM3 | LCD backlight PWM |
| USB OTG FS | HID Consumer Control |
| USART2 | Debug output |
| GPIO PB13/14/15 | Rotary encoder SW/DT/CLK |
| GPIO PA11/PA12 | USB D−/D+ |


## Roadmap

- [x] SPI LCD with custom volume dial UI
- [x] Rotary encoder input with debounce
- [x] USB HID Consumer Control — PC volume control
- [ ] Sync LCD volume from PC via UART
- [ ] Mute toggle on encoder button press
- [ ] Custom PCB

## Development Environment

- STM32CubeIDE
- STM32 HAL (CubeMX generated)
- Waveshare LCD library (adapted for STM32F401RE)
