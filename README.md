# XRigger — Smart Electronic Downrigger System

![Status](https://img.shields.io/badge/status-in%20development-yellow)
![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Power](https://img.shields.io/badge/power-Ryobi%2040V-orange)
![License](https://img.shields.io/badge/license-MIT-green)

XRigger is a DIY smart electronic downrigger built around a **Ryobi 40V battery platform**, a **BLDC motor drive system**, and an **ESP32 microcontroller**. It replaces the analog knobs and limited 12V power of commercial downriggers with digital depth control, smooth motor ramping, Bluetooth app connectivity, and a physical on-unit interface.

---

## Features

- **Electronic depth control** with configurable set-points
- **Smooth motor ramp up / ramp down** to prevent line snap
- **UP / DOWN direction control** with auto-stop at limits
- **Zero calibration** (surface reference point)
- **Smart UI** — physical buttons + Bluetooth app
- **40V power platform** — 3–4× torque headroom vs 12V competitors
- **Battery-independent** from boat electrical system

---

## System Architecture

```
Ryobi 40V Battery
       │
   [20A Fuse]
       │
   ┌───┴────────────────────────────┐
   │                                │
[Buck 40V→5V]               [JKBLD300 V2]  ← 40V direct
   │                                │
[ESP32]  ──── PWM / GPIO ──────────►│
   │                                │
   └────── Encoder feedback ◄───────┤
                                    │
                               [BLDC Motor]
                                    │
                               [RV30 Gearbox]
                                    │
                            [Downrigger Spool]
```

---

## Hardware

| Component | Part / Model | Notes |
|---|---|---|
| Battery | Ryobi 40V Li-Ion | Platform standard |
| BLDC Motor | 57DMW or 90DMW class | 3-phase |
| Motor Driver | JKBLD300 V2 | 14–56V, 15A max |
| Gearbox | RV30 Gear Reduction | Torque multiplication |
| MCU | ESP32-WROOM-32 | Wi-Fi + BLE |
| Encoder | Rotary encoder / Hall effect | Depth tracking |
| Display | OLED or TFT (I2C/SPI) | Depth & status |
| Buttons | Momentary tactile switches | UP / DOWN / SET |
| Speed Pot | 10kΩ potentiometer | Optional manual speed |
| Limit Switches | Mechanical or magnetic | Upper/lower bounds |
| Fusing | 20A inline fuse holder | Battery protection |
| Wiring | 14–18 AWG silicone wire | High-current runs |
| Enclosure | IP65 rated enclosure | Waterproofing |

---

## Wiring Summary

### ESP32 → JKBLD300 Pin Mapping

| ESP32 GPIO | JKBLD300 Terminal | Function | Logic |
|---|---|---|---|
| GPIO25 | SV | PWM speed (via RC filter + divider) | 0–5V analog |
| GPIO26 | EN | Enable motor | LOW = run |
| GPIO27 | F/R | Direction | HIGH = down, LOW = up |
| GPIO14 | BRK | Brake | HIGH = brake, LOW = run |
| GPIO34 | ALM | Fault monitor (input) | 5V = OK, 0V = fault |
| GND | COM | Common ground | — |

### ⚠️ Important Notes
- ESP32 is **3.3V logic**. Use an RC low-pass filter (10kΩ + 10µF) on GPIO25 before the SV pin.
- ALM pin outputs **5V** — use a voltage divider (3.3kΩ / 1.8kΩ) before GPIO34.
- Always **ramp down to 0** before changing direction to protect the gearbox.
- Connect motor U/V/W phases to the JKBLD300 motor terminals.

---

## Project Phases

| Phase | Deliverable | Status |
|---|---|---|
| Phase 1 | Hardware Architecture & BOM | ✅ Complete |
| Phase 2 | Wiring Schematic & Power Design | 🔄 In Progress |
| Phase 3 | ESP32 Firmware — Motor Control Core | 🔄 In Progress |
| Phase 4 | ESP32 Firmware — Depth & Encoder Logic | 📋 Planned |
| Phase 5 | Physical UI — Buttons, Display, Enclosure | 📋 Planned |
| Phase 6 | Bluetooth App Development | 📋 Planned |
| Phase 7 | Integration Testing & Field Trials | 📋 Planned |
| Phase 8 | Documentation & Final Build | 📋 Planned |

---

## Repository Structure

```
xrigger/
├── README.md
├── firmware/
│   └── motor_control/
│       └── motor_control.ino    ← ESP32 motor control firmware
├── docs/
│   ├── wiring_schematic.md      ← Full pin mapping & signal descriptions
│   ├── project_plan.md          ← Full project plan & technical spec
│   └── bom.md                   ← Bill of materials
└── hardware/
    └── notes.md                 ← Hardware notes & design decisions
```

---

## Getting Started

### Prerequisites
- Arduino IDE 2.x or ESP-IDF
- ESP32 board package installed
- JKBLD300 V2 motor driver
- ESP32-WROOM-32 development board

### Upload Firmware
1. Open `firmware/motor_control/motor_control.ino` in Arduino IDE
2. Select board: **ESP32 Dev Module**
3. Set upload speed: **115200**
4. Upload and open Serial Monitor at 115200 baud

### First Test Sequence
The default `loop()` runs a test cycle:
- Deploy line **DOWN** at speed 150 for 3 seconds
- Stop
- Retrieve **UP** at speed 180 for 3 seconds
- Stop

Replace with your keypad/UI logic once motor control is verified.

---

## Risks & Mitigations

| Risk | Mitigation |
|---|---|
| Motor overheat at high load | Current monitoring via ALM pin; thermal cutoff logic |
| Encoder slip / depth inaccuracy | Periodic re-zero from physical spool reference |
| BLE range on water | Physical controls always primary; BLE as convenience layer |
| Water ingress | IP65+ enclosure; sealed cable glands; conformal coat PCB |
| Battery voltage sag under load | ADC voltage monitoring; low-voltage cutoff |
| Direction reversal damage | Firmware ramp-down before direction switch |

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

## Author

XRigger Project — DIY Smart Downrigger System  
Built with ESP32 + Ryobi 40V + JKBLD300 V2
