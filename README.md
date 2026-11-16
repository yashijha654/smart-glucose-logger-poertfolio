# Smart Glucose Logger

A low-cost ESP32 prototype that combines threshold alerts for glucose readings with scheduled reminders. The breadboard model uses a DS3231 real-time clock, a 128 × 64 OLED, two buttons, two LEDs, and an active buzzer.

> **Prototype status:** The firmware currently cycles through simulated values. No glucose sensor is connected. This project is for embedded-systems learning and is not validated for medical or treatment use.

## Project at a glance

- **Controller:** ESP32-WROOM development board
- **Clock:** DS3231 RTC over I²C
- **Display:** SSD1306 128 × 64 OLED over I²C
- **Alerts:** red LED and buzzer; green LED for the normal state
- **Inputs:** Acknowledge and Log push buttons
- **Thresholds in the demo:** below 70 mg/dL and above 180 mg/dL
- **Dose reminders in the demo:** 08:00 and 20:00
- **Current stage:** breadboard behavior demonstrated; KiCad PCB routing remains in progress

## What it does

The sketch selects a new test value every five seconds. Values outside the configured limits latch the glucose alarm. The RTC is checked for the scheduled reminder times. While either alert is active, the red LED and buzzer blink; otherwise, the green LED is on. The OLED displays the time, reading, status, and a `SIMULATED DATA` label.

Press **Ack** to clear the glucose alarm and dose reminder. Press **Log** to print the current value and timestamp to the Serial Monitor. Logs are not stored after the serial session ends.

## Hardware and pin map

| Function | ESP32 GPIO |
|---|---:|
| I²C SDA | 21 |
| I²C SCL | 22 |
| Red LED | 19 |
| Green LED | 18 |
| Active buzzer | 17 |
| Acknowledge button | 5 |
| Log button | 4 |

The buttons use the ESP32 internal pull-ups and read LOW when pressed. The RTC uses I²C address `0x68`; the OLED is typically at `0x3C`.

## Repository layout

```text
.
├── index.html                     # Portfolio site
├── 404.html                       # Branded not-found page
├── assets/                        # Build photos and working-model video
└── firmware/
    └── SmartGlucoseLogger.ino     # Complete Arduino sketch
```

## View the portfolio locally

Open `index.html` in a browser. The page has no build step or package dependencies. Keep the `assets/` and `firmware/` directories beside `index.html`; the page uses relative paths to load its media and source code.

## Firmware

Open [`firmware/SmartGlucoseLogger.ino`](firmware/SmartGlucoseLogger.ino) in the Arduino IDE. Install the Arduino libraries **RTClib**, **Adafruit GFX Library**, and **Adafruit SSD1306**, select the correct ESP32 board and port, then upload the sketch.

The firmware prints serial output at 115200 baud. On first RTC power-up, set the DS3231 time; the sketch adjusts it to its compile time only when `rtc.lostPower()` is true.

### Implementation notes

- `millis()` schedules five-second readings and 250 ms alert blinking, while the loop continues to check the buttons, clock, and display.
- Out-of-range readings latch `alarmOn`; the next in-range reading does not clear the alert.
- The dose reminder guard stores hour and minute but not the date. As written, the same reminder time will not retrigger on later days; use a date-aware key for daily reminders.
- The display and RTC share I²C. Confirm the final KiCad schematic matches the breadboard pin map before fabricating the PCB.

## Current limitations and next steps

This version has no physical glucose sensor, persistent logging, wireless connection, or remote alerts. Alarm thresholds and reminder times are compiled into the sketch. The PCB layout needs routing and an outline.

1. Align and review the schematic against the working wiring.
2. Complete PCB routing and add the board outline.
3. Add flash or SD-card storage for timestamped readings.
4. Make alarm thresholds and reminder times configurable.
5. Integrate a suitable calibrated sensor only after defining its electrical interface and a measurement validation plan.

## Project team

**Department:** B.Tech Biomedical Engineering (Core)

- Yashi Jha — RA2411013010080
- Pragya Shukla — RA2411013010084
- S. Syed Thahir — RA2411013010072
- V. Kamalesh — RA2411013010111

## Technical references

- Espressif Systems, *ESP32 Series Datasheet*.
- Analog Devices, *DS3231 Extremely Accurate I²C-Integrated RTC/TCXO/Crystal Datasheet*.
- Solomon Systech, *SSD1306 128 × 64 Dot Matrix OLED Driver Datasheet*.
- NXP Semiconductors, *I²C-bus specification and user manual (UM10204)*.
- KiCad official documentation.
