// ============================================================
// XRigger Motor Control Core
// ESP32-WROOM-32 → JKBLD300 V2 Brushless Motor Driver
//
// Controls: Speed (PWM), Direction (F/R), Enable (EN),
//           Brake (BRK), Fault monitor (ALM)
//
// Power: Ryobi 40V → JKBLD300 (direct)
//        Ryobi 40V → Buck converter → 5V → ESP32 VIN
//
// Wiring:
//   GPIO25 → RC filter → voltage divider → SV (speed)
//   GPIO26 → EN  (LOW = run)
//   GPIO27 → F/R (HIGH = down/forward, LOW = up/reverse)
//   GPIO14 → BRK (HIGH = brake, LOW = run)
//   GPIO34 ← ALM (5V = OK, 0V = fault) via voltage divider
// ============================================================

#include <Arduino.h>

// ------------------------------------------------------------
// Pin Definitions
// ------------------------------------------------------------
#define PIN_PWM_SPEED   25    // PWM speed output → SV pin
#define PIN_ENABLE      26    // EN: LOW = run, HIGH = coast stop
#define PIN_DIRECTION   27    // F/R: HIGH = down, LOW = up (retrieve)
#define PIN_BRAKE       14    // BRK: HIGH = brake, LOW = run
#define PIN_ALARM       34    // ALM input: 5V = OK, 0V = fault
                              // ⚠️ Use 3.3kΩ/1.8kΩ divider before this pin

// ------------------------------------------------------------
// PWM Configuration
// ------------------------------------------------------------
#define PWM_CHANNEL     0
#define PWM_FREQ        5000  // 5kHz — smooth, above audible range
#define PWM_RESOLUTION  8     // 8-bit resolution: 0–255

// ------------------------------------------------------------
// Speed & Ramp Settings
// ------------------------------------------------------------
#define SPEED_MIN       0
#define SPEED_MAX       200   // Cap below 255 for safety headroom
#define SPEED_DEFAULT   150   // Default run speed
#define RAMP_STEP       3     // PWM units per ramp step
#define RAMP_DELAY_MS   20    // ms between ramp steps (~60ms 0→full)

// ------------------------------------------------------------
// Timing
// ------------------------------------------------------------
#define DIRECTION_SETTLE_MS   300   // Wait after stop before reversing
#define BRAKE_SETTLE_MS       100   // Wait after brake before other ops

// ------------------------------------------------------------
// State
// ------------------------------------------------------------
int  currentSpeed    = 0;
bool motorEnabled    = false;
bool faultDetected   = false;

// Direction enum
enum Direction { DOWN, UP };
Direction currentDirection = DOWN;


// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("XRigger Motor Controller — Initializing...");

  // Control outputs
  pinMode(PIN_ENABLE,    OUTPUT);
  pinMode(PIN_DIRECTION, OUTPUT);
  pinMode(PIN_BRAKE,     OUTPUT);

  // Fault input (5V via divider — safe for 3.3V GPIO)
  pinMode(PIN_ALARM, INPUT);

  // Safe state on boot: brake on, motor disabled
  digitalWrite(PIN_BRAKE,     HIGH);  // Brake engaged
  digitalWrite(PIN_ENABLE,    HIGH);  // Motor disabled
  digitalWrite(PIN_DIRECTION, HIGH);  // Default direction: DOWN

  // Configure PWM channel
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM_SPEED, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);

  Serial.println("XRigger Ready.");
  printStatus();
}


// ============================================================
// FAULT MONITORING
// Returns true if fault detected (motor will be stopped)
// ============================================================
bool checkFault() {
  if (digitalRead(PIN_ALARM) == LOW) {
    if (!faultDetected) {
      Serial.println("⚠️  FAULT DETECTED on ALM pin — stopping motor!");
      faultDetected = true;
      emergencyStop();
    }
    return true;
  }
  faultDetected = false;
  return false;
}


// ============================================================
// EMERGENCY STOP
// Immediate halt — no ramp. Use for faults and limit switches.
// ============================================================
void emergencyStop() {
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(PIN_BRAKE, HIGH);   // Hard brake
  digitalWrite(PIN_ENABLE, HIGH);  // Disable driver
  currentSpeed  = 0;
  motorEnabled  = false;
  Serial.println("🛑 Emergency Stop");
}


// ============================================================
// SMOOTH RAMP UP
// Ramps from current speed to targetSpeed in controlled steps.
// Sets direction before enabling motor.
// ============================================================
void rampUp(int targetSpeed, Direction dir) {
  if (checkFault()) return;

  targetSpeed = constrain(targetSpeed, SPEED_MIN, SPEED_MAX);

  // Set direction BEFORE enabling — never change while running
  setDirection(dir);
  delay(50);  // Brief settle after direction set

  // Release brake, enable motor
  digitalWrite(PIN_BRAKE,  LOW);
  digitalWrite(PIN_ENABLE, LOW);
  motorEnabled = true;

  // Ramp up
  while (currentSpeed < targetSpeed) {
    if (checkFault()) return;
    currentSpeed = min(currentSpeed + RAMP_STEP, targetSpeed);
    ledcWrite(PWM_CHANNEL, currentSpeed);
    delay(RAMP_DELAY_MS);
  }

  Serial.printf("▶  Running — speed: %d, direction: %s\n",
    currentSpeed, (dir == DOWN) ? "DOWN" : "UP");
}


// ============================================================
// SMOOTH RAMP DOWN & STOP
// Graceful deceleration before stopping.
// ============================================================
void rampStop() {
  if (currentSpeed == 0 && !motorEnabled) return;

  Serial.println("⏹  Ramping down...");

  while (currentSpeed > 0) {
    currentSpeed = max(currentSpeed - RAMP_STEP, 0);
    ledcWrite(PWM_CHANNEL, currentSpeed);
    delay(RAMP_DELAY_MS);
  }

  delay(BRAKE_SETTLE_MS);
  digitalWrite(PIN_BRAKE,  HIGH);  // Apply brake
  digitalWrite(PIN_ENABLE, HIGH);  // Disable driver
  motorEnabled = false;

  Serial.println("✅ Stopped");
}


// ============================================================
// SET SPEED (while running)
// Adjusts speed smoothly without stopping the motor.
// ============================================================
void setSpeed(int targetSpeed) {
  if (!motorEnabled) return;
  if (checkFault()) return;

  targetSpeed = constrain(targetSpeed, SPEED_MIN, SPEED_MAX);

  if (targetSpeed == 0) {
    rampStop();
    return;
  }

  int step = (targetSpeed > currentSpeed) ? RAMP_STEP : -RAMP_STEP;

  while (currentSpeed != targetSpeed) {
    if (checkFault()) return;
    currentSpeed += step;
    currentSpeed = constrain(currentSpeed, SPEED_MIN, targetSpeed);
    ledcWrite(PWM_CHANNEL, currentSpeed);
    delay(RAMP_DELAY_MS);
  }
}


// ============================================================
// SET DIRECTION
// Always stop before changing direction to protect gearbox.
// ============================================================
void setDirection(Direction newDir) {
  if (motorEnabled && newDir != currentDirection) {
    rampStop();
    delay(DIRECTION_SETTLE_MS);
  }
  currentDirection = newDir;
  digitalWrite(PIN_DIRECTION, (newDir == DOWN) ? HIGH : LOW);
}


// ============================================================
// JOG
// Run motor for a specified duration then stop.
// ============================================================
void jog(Direction dir, int speed, unsigned long durationMs) {
  Serial.printf("↕  Jog %s for %lums at speed %d\n",
    (dir == DOWN) ? "DOWN" : "UP", durationMs, speed);
  rampUp(speed, dir);
  delay(durationMs);
  rampStop();
}


// ============================================================
// STATUS PRINT
// ============================================================
void printStatus() {
  Serial.println("--- XRigger Status ---");
  Serial.printf("  Motor enabled : %s\n", motorEnabled ? "YES" : "NO");
  Serial.printf("  Current speed : %d / %d\n", currentSpeed, SPEED_MAX);
  Serial.printf("  Direction     : %s\n", (currentDirection == DOWN) ? "DOWN" : "UP");
  Serial.printf("  Fault state   : %s\n", faultDetected ? "FAULT" : "OK");
  Serial.printf("  ALM pin       : %s\n", digitalRead(PIN_ALARM) ? "OK (5V)" : "FAULT (0V)");
  Serial.println("----------------------");
}


// ============================================================
// MAIN LOOP — Demo/Bench Test Sequence
// Replace this with your keypad / depth control logic
// ============================================================
void loop() {
  Serial.println("\n=== XRigger Bench Test Cycle ===");

  // Check for faults before doing anything
  if (checkFault()) {
    Serial.println("Fault active — waiting for clear...");
    delay(2000);
    return;
  }

  // Test 1: Deploy DOWN
  Serial.println("Test 1: Deploy DOWN");
  rampUp(SPEED_DEFAULT, DOWN);
  delay(3000);

  // Test 2: Stop
  Serial.println("Test 2: Stop");
  rampStop();
  delay(1000);

  // Test 3: Retrieve UP
  Serial.println("Test 3: Retrieve UP");
  rampUp(180, UP);
  delay(3000);

  // Test 4: Ramp stop
  rampStop();
  printStatus();
  delay(3000);
}
