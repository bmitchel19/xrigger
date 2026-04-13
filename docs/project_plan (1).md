# XRigger — Project Plan & Technical Documentation

Rev 1.0 | 2026

---

## 1. Project Overview

XRigger is a DIY smart electronic downrigger built around a Ryobi 40V battery platform, a BLDC motor drive system, and an ESP32 microcontroller. It replaces the analog knobs and limited 12V power of commercial downriggers with digital depth control, smooth motor ramping, Bluetooth app connectivity, and a physical on-unit interface.

The system is designed for freshwater and saltwater fishing applications where precise lure depth management is critical. By leveraging 40V power, XRigger delivers higher torque headroom than 12V competitors while remaining portable and battery-independent from the boat's electrical system.

### Core Goals

- Electronic depth control with configurable set-points
- Smooth motor ramp up / ramp down to prevent line snap
- UP / DOWN direction control with auto-stop at limits
- Zero calibration (surface reference point)
- Smart user interface — both physical buttons and Bluetooth app

---

## 2. System Architecture

### 2.1 Power Platform

| Parameter | Value |
|---|---|
| Battery | Ryobi 40V Li-Ion platform |
| Voltage Range | 36V–42V nominal (14V–56V driver supported) |
| Advantage | 3–4× power headroom vs. 12V downriggers |
| Portability | No boat electrical system dependency |

### 2.2 Drive System

| Parameter | Value |
|---|---|
| Motor | BLDC 57DMW / 90DMW class, 3-Phase DC Brushless |
| Motor Controller | JKBLD300 V2 — 14V–56V input, max 15A output |
| Control Methods | PWM (ESP32), external potentiometer, analog voltage |
| Speed Range | 0–20,000 RPM (regulated by controller) |
| Gear Reduction | RV30 gearbox for torque multiplication |
| Power Rating | < 300W (driver limit) |
| Special Features | Large-torque start, fast braking, FWD/REV switching |

### 2.3 Control System

| Parameter | Value |
|---|---|
| MCU | ESP32-WROOM-32 Development Board |
| Control Mode | Digital (PWM) — not analog knobs |
| Connectivity | Wi-Fi + Bluetooth 4.2 (BLE) built-in |
| Hall Signals | 5V, 12mA drive current (JKBLD300 compatible) |
| Potentiometer | 10kΩ external (speed reference, if used) |
| Programming | Arduino IDE / ESP-IDF |

---

## 3. Functional Requirements

### 3.1 Motor Control

- PWM output from ESP32 to JKBLD300 V2 for variable speed
- Soft start: ramp from 0% to target duty cycle over configurable time (1–3 sec)
- Soft stop: ramp down before direction change or halt
- Direction control: GPIO signals to driver FWD/REV pins
- Emergency stop: immediate cut on limit switch trigger

### 3.2 Depth Management

- Rotary encoder or hall-effect counter on spool shaft for line length tracking
- Zero calibration: press-and-hold to set current position as surface (0 ft)
- Set-point entry: dial or app input to target depth
- Auto-stop: motor halts when encoder count matches target depth
- Depth display: shown on OLED/TFT screen and in app

### 3.3 Physical UI (On-Unit)

- UP button: raises rig at configured speed
- DOWN button: lowers rig at configured speed
- SET / ZERO button: calibration and depth set-point entry
- Speed dial (optional): 10kΩ potentiometer mapped to PWM duty
- Display: OLED or small TFT showing depth, speed %, status
- LED indicators: power, direction, fault

### 3.4 Bluetooth App

- BLE connection to ESP32 (no internet required on the water)
- Real-time depth readout on phone screen
- Set target depth — rig auto-runs and stops
- Manual UP / DOWN controls with speed slider
- Zero calibration trigger from app
- Battery voltage monitoring (ADC → app)
- Saved presets: store favorite depth settings by species/location

---

## 4. Project Phases & Timeline

| Phase | Deliverable | Est. Duration | Status |
|---|---|---|---|
| Phase 1 | Hardware Architecture & BOM Finalization | 1 week | ✅ Complete |
| Phase 2 | Wiring Schematic & Power Design | 1–2 weeks | 🔄 In Progress |
| Phase 3 | ESP32 Firmware — Motor Control Core | 2–3 weeks | 🔄 In Progress |
| Phase 4 | ESP32 Firmware — Depth & Encoder Logic | 1–2 weeks | 📋 Planned |
| Phase 5 | Physical UI — Buttons, Display, Enclosure | 2 weeks | 📋 Planned |
| Phase 6 | Bluetooth App Development | 3–4 weeks | 📋 Planned |
| Phase 7 | Integration Testing & Field Trials | 2 weeks | 📋 Planned |
| Phase 8 | Documentation & Final Build | 1 week | 📋 Planned |

---

## 5. Bill of Materials

| Component | Part / Model | Notes |
|---|---|---|
| Battery | Ryobi 40V Li-Ion | Platform standard |
| BLDC Motor | 57DMW or 90DMW class | 3-phase |
| Motor Driver | JKBLD300 V2 | 14–56V, 15A max |
| Gearbox | RV30 Gear Reduction | Torque multiplication |
| MCU | ESP32-WROOM-32 | Wi-Fi + BLE |
| Encoder | Rotary encoder / Hall effect | Depth tracking |
| Display | OLED or TFT (I2C/SPI) | Depth & status |
| Buttons | Momentary tactile switches | UP/DOWN/SET |
| Speed Pot | 10kΩ potentiometer | Optional manual speed |
| Limit Switches | Mechanical or magnetic | Upper/lower bounds |
| Buck Converter | 40V→5V, 1A+ (LM2596 or similar) | ESP32 logic power |
| RC Filter | 10kΩ resistor + 10µF cap | PWM smoothing for SV pin |
| ALM Divider | 3.3kΩ + 1.8kΩ resistors | 5V→3.3V for GPIO34 |
| Op-Amp (optional) | MCP6001 (rail-to-rail) | Full 0–5V speed range on SV |
| Fusing | 20A inline fuse + 58V rated holder | Battery protection |
| Wiring | 14–18 AWG silicone wire | High-current runs |
| Enclosure | IP65 rated enclosure | Waterproofing |
| Cable Glands | Sized to wiring harness | Sealed entries |

---

## 6. Risks & Mitigations

| Risk | Mitigation |
|---|---|
| Motor overheat at high load | Current monitoring via driver feedback; thermal cutoff logic in firmware |
| Encoder slip / depth inaccuracy | Periodic re-zero from physical spool reference; hall-effect backup |
| BLE range on water (reflections) | Physical controls always primary; BLE as convenience layer only |
| Water ingress to enclosure | IP65+ enclosure; sealed cable glands; conformal coat PCB |
| Battery voltage sag under load | ADC voltage monitoring; low-voltage cutoff to protect cells |
| Direction reversal damage | Firmware ramp-down before direction switch; driver braking mode |

---

## 7. Immediate Next Steps

- [x] Finalize wiring schematic: ESP32 ↔ JKBLD300 V2 signal mapping
- [x] Prototype motor control firmware: PWM ramp in/out, direction switching, fault monitor
- [ ] Build RC filter + ALM voltage divider on breadboard
- [ ] Bench test motor control firmware with JKBLD300 and motor
- [ ] Select encoder type and mount point on spool shaft
- [ ] Choose display (OLED I2C recommended for simplicity)
- [ ] Draft BLE GATT profile for app ↔ ESP32 communication
- [ ] Select app development approach: React Native (cross-platform) or native iOS/Android
- [ ] Source IP65 enclosure and plan cable gland positions

---

## 8. JKBLD300 V2 Driver Reference

### Electrical Specifications

| Parameter | Min | Typical | Max | Unit |
|---|---|---|---|---|
| Voltage Input DC | 14 | 48 | 56 | V |
| Output current | — | — | 15 | A |
| Motor speed range | 0 | — | 20000 | RPM |
| Hall signal voltage | — | — | 5 | V |
| Hall drive current | 12 | — | — | mA |
| External potentiometer | — | 10 | — | kΩ |

### Control Signal Logic

| Terminal | Disconnected / HIGH | LOW / Shorted to COM |
|---|---|---|
| BRK | Motor brakes | Motor runs |
| EN | Motor coasts to stop | Motor runs |
| F/R | Forward rotation (DOWN) | Reverse rotation (UP) |
| SV | Speed reference (0–5V or PWM) | — |

### Output Signals

| Terminal | Description |
|---|---|
| SPEED | Pulse frequency proportional to motor RPM. Formula: N = (F/P) × 60/3 |
| ALM | 5V normally, 0V on fault |
