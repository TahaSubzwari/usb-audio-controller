\# USB Audio Controller



A hardware volume controller built on the STM32F401RE (ARM Cortex-M4).

Turn a physical knob to control volume — displayed in real time on a 2-inch SPI LCD.

USB HID integration (for actual PC volume control) is in active development.



\## Features



\- Real-time potentiometer input via ADC

\- Volume mapped to 0–100 scale

\- Custom arc-based volume dial rendered on 2-inch SPI LCD

\- Throttled display refresh to prevent flicker



\## Hardware



| Component | Details |

|---|---|

| Microcontroller | STM32 Nucleo F401RE |

| Display | Waveshare 2-inch SPI LCD (240×240) |

| Input | 10kΩ potentiometer |



\## Roadmap



\- \[x] ADC potentiometer input

\- \[x] SPI LCD with custom volume dial UI

\- \[ ] USB HID — PC volume control via HID consumer report

\- \[ ] Audio device switching

\- \[ ] Custom PCB



\## Development Environment



\- STM32CubeIDE

\- STM32 HAL (auto-generated via CubeMX)

\- Waveshare LCD library (adapted for STM32F401RE)

