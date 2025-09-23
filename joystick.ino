#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm(0x40);

// Channels (your mapping)
const int ch_eyeLR = 0;
const int ch_eyeUD = 1;
const int ch_lidBL = 3;
const int ch_lidTL = 2;
const int ch_lidTR = 4;
const int ch_lidBR = 5;

// Eye calibration
int centerLR = 450;
int centerUD = 450;
int travelLR = 80;   // normal swing
int travelUD = 80;

// Absolute safety limits (mechanical stops)
int maxTravelLR = 150;  // never go beyond ±150 from center
int maxTravelUD = 150;

// Lid calibration
int centerLids = 450;
int travelLids = 150;

// Directions from calibration
int dirBL = +1;
int dirTL = -1;
int dirTR = +1;
int dirBR = -1;

// Joystick pins
const int pinJoyX = A0;
const int pinJoyY = A1;
const int pinJoyBtn = 7;

// Last positions to avoid buzzing
int lastPulseX = -1;
int lastPulseY = -1;

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(60);
  delay(10);

  pinMode(pinJoyBtn, INPUT_PULLUP);

  // Eyes start centered
  pwm.setPWM(ch_eyeLR, 0, centerLR);
  pwm.setPWM(ch_eyeUD, 0, centerUD);

  // Lids start OPEN
  openLids();
}

void loop() {
  // --- Eyes from joystick ---
  int rawX = analogRead(pinJoyX);
  int rawY = analogRead(pinJoyY);

  float normX = (rawX - 512) / 512.0;
  float normY = (rawY - 512) / 512.0;

  int pulseX = centerLR + normX * travelLR;
  int pulseY = centerUD - normY * travelUD;

  // Apply mechanical clamps
  pulseX = constrain(pulseX, centerLR - maxTravelLR, centerLR + maxTravelLR);
  pulseY = constrain(pulseY, centerUD - maxTravelUD, centerUD + maxTravelUD);

  // Only update if changed enough
  if (abs(pulseX - lastPulseX) > 2) {
    pwm.setPWM(ch_eyeLR, 0, pulseX);
    lastPulseX = pulseX;
  }
  if (abs(pulseY - lastPulseY) > 2) {
    pwm.setPWM(ch_eyeUD, 0, pulseY);
    lastPulseY = pulseY;
  }

  // --- Blink when button pressed ---
  if (digitalRead(pinJoyBtn) == LOW) {
    blink();
    delay(300); // debounce
  }

  delay(15);
}

// --- Helpers ---

void closeLids() {
  pwm.setPWM(ch_lidBL, 0, centerLids);
  pwm.setPWM(ch_lidTL, 0, centerLids);
  pwm.setPWM(ch_lidTR, 0, centerLids);
  pwm.setPWM(ch_lidBR, 0, centerLids);
}

void openLids() {
  pwm.setPWM(ch_lidBL, 0, centerLids + dirBL * travelLids);
  pwm.setPWM(ch_lidTL, 0, centerLids + dirTL * travelLids);
  pwm.setPWM(ch_lidTR, 0, centerLids + dirTR * travelLids);
  pwm.setPWM(ch_lidBR, 0, centerLids + dirBR * travelLids);
}

void blink() {
  closeLids();
  delay(150);
  openLids();
}
