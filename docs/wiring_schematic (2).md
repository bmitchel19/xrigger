# XRigger — Wiring Schematic & Signal Reference

Rev 1.0 | Phase 2

---

## Power Architecture

```
Ryobi 40V Li-Ion Battery
         │
     [20A Fuse]  ← 58V rated, inline before any split
         │
    ┌────┴─────────────────────────────────────┐
    │                                          │
[Buck Converter]                        [JKBLD300 V2]
 40V → 5V / 1A+                          DC+ (40V direct)
    │                                    DC− (Battery GND)
[ESP32 VIN 5V]
[ESP32 GND]
```

**Wire gauge:**
- Battery → Fuse → JKBLD300 DC+/DC−: **14–16 AWG silicone** (carries full motor current up to 15A)
- Battery → Buck converter: **18 AWG** (1A logic supply only)
- All signal lines (GPIO → driver): **22–24 AWG**

---

## ESP32 → JKBLD300 Signal Mapping

| ESP32 GPIO | JKBLD300 Terminal | Signal Type | Logic Level | Notes |
|---|---|---|---|---|
| GPIO25 | SV | PWM Speed | 0–5V analog | Via RC filter + voltage divider |
| GPIO26 | EN | Enable | LOW = run | HIGH/float = coast stop |
| GPIO27 | F/R | Direction | HIGH = DOWN, LOW = UP | Set before enabling |
| GPIO14 | BRK | Brake | HIGH = brake, LOW = run | HIGH/float = safe braked state |
| GPIO34 | ALM | Fault input | 5V = OK, 0V = fault | Needs voltage divider (5V→3.3V) |
| GND | COM | Common ground | — | Tie ESP32 GND to JKBLD300 COM |

---

## PWM Speed Signal Circuit (GPIO25 → SV)

The JKBLD300 SV pin expects a 0–5V analog signal.  
The ESP32 outputs 3.3V PWM. Two stages are needed:

### Stage 1: RC Low-Pass Filter (PWM → Smooth Analog)
```
GPIO25 ──[10kΩ]──┬── To Stage 2
                 │
               [10µF]
                 │
               GND
```
- Converts 5kHz PWM to a smooth DC voltage
- Output range: 0V – 3.3V (proportional to duty cycle)

### Stage 2: Voltage Divider (3.3V → ~5V scale)
> Note: A simple divider cannot boost voltage above source (3.3V).  
> Option A: Accept 0–3.3V range on SV pin (motor runs 0–66% of max RPM). Simplest approach.  
> Option B: Use a small op-amp (MCP6001, rail-to-rail) powered from 5V buck output to buffer and scale to 0–5V. Recommended for full speed range.

**Option A (resistor divider for impedance matching only):**
```
RC Output ──[1kΩ]── SV pin
                │
             [10kΩ]  (matches JKBLD300 external pot spec)
                │
               GND
```

**Option B (op-amp unity buffer):**
```
RC Output ──[+IN]──[MCP6001]──[OUT]── SV pin
              │
           GND (−IN)
```
Op-amp powered from 5V buck output.

---

## ALM Fault Signal Circuit (ALM → GPIO34)

ALM pin outputs 5V normally, drops to 0V on fault.  
ESP32 GPIO34 is input-only and 3.3V max.

```
JKBLD300 ALM ──[3.3kΩ]──┬── GPIO34
                         │
                       [1.8kΩ]
                         │
                        GND
```

Divider output at 5V input: `5V × 1.8/(3.3+1.8)` = **~1.76V** → reads HIGH on ESP32 ✅  
Divider output at 0V input: **0V** → reads LOW on ESP32 ✅

---

## Hall Sensor Connections (Motor → JKBLD300)

| Motor Wire | JKBLD300 Terminal | Notes |
|---|---|---|
| Hall HU | HU | Motor Hall signal phase U |
| Hall HV | HV | Motor Hall signal phase V |
| Hall HW | HW | Motor Hall signal phase W |
| Hall +5V | REF+ | Hall sensor 5V supply |
| Hall GND | REF− | Hall sensor ground |

---

## Motor Phase Connections (JKBLD300 → BLDC Motor)

| JKBLD300 Terminal | Motor Wire | Notes |
|---|---|---|
| U | U phase | Match motor documentation |
| V | V phase | If motor runs backwards, swap any two phases |
| W | W phase | |

**If motor spins wrong direction:** Swap any two of U/V/W — do not use F/R pin alone to fix phase wiring.

---

## JKBLD300 Control Logic Summary

| Terminal | Disconnected / HIGH | LOW / Shorted to COM |
|---|---|---|
| EN | Motor coasts to stop | Motor runs |
| BRK | Motor brakes | Motor runs |
| F/R | Motor forward (DOWN) | Motor reverse (UP) |

> **Safe default:** All control pins float HIGH → motor is braked and disabled. This is the correct power-on state.

---

## SPEED Output (Optional — Depth Counting)

The JKBLD300 SPEED terminal outputs a pulse frequency proportional to motor RPM.

**Formula:**
```
N (rpm) = (F / P) × 60 / 3

Where:
  F = pulse frequency (Hz)
  P = number of motor poles
  N = motor speed (RPM)
```

Connect to a spare ESP32 GPIO with interrupt for pulse counting.  
Use this as a secondary depth reference alongside the spool encoder.

---

## Wiring Checklist

- [ ] Battery negative tied to JKBLD300 DC− and ESP32 GND (common ground)
- [ ] 20A fuse installed on positive rail before any branch
- [ ] RC filter on GPIO25 before SV pin
- [ ] Voltage divider on ALM before GPIO34
- [ ] Hall sensor wires connected REF+ / HU / HV / HW / REF−
- [ ] Motor U/V/W phases connected (verify rotation direction on first test)
- [ ] All high-current connections crimped or soldered — no push connectors on 40V rail
- [ ] Enclosure cable glands sealed
- [ ] Conformal coating on ESP32 and any exposed PCB
