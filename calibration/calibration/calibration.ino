#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Mapping: [eyeLR, eyeUD, lidBL, lidTL, lidTR, lidBR]
const int ch[6] = {0, 1, 3, 2, 4, 5};
const char* name[6] = {
  "eyeLR",   // ch0
  "eyeUD",   // ch1
  "lidBL",   // ch3
  "lidTL",   // ch2
  "lidTR",   // ch4
  "lidBR"    // ch5
};

// Each servo has: center, travel, direction (+1 or -1)
int centerPulse[6] = {450, 450, 450, 450, 450, 450};
int travelPulse[6] = {6, 6, 6, 6, 6, 6};
int direction[6]   = {-1, -1, -1, -1, -1, -1};

int selected = 0;

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(60);
  delay(10);

  for (int i = 0; i < 6; i++) {
    pwm.setPWM(ch[i], 0, centerPulse[i]);
  }

  Serial.println(F("=== 6-servo Calibrator (setc/sett + direction) ==="));
  Serial.println(F("Commands:"));
  Serial.println(F("  s N     -> select servo (0..5)"));
  Serial.println(F("  + / -   -> adjust center by 1 tick"));
  Serial.println(F("  > / <   -> adjust travel by 1 tick"));
  Serial.println(F("  setc V  -> set center to V ticks"));
  Serial.println(F("  sett V  -> set travel to V ticks"));
  Serial.println(F("  dir +   -> set direction = +1 (open = center + travel)"));
  Serial.println(F("  dir -   -> set direction = -1 (open = center - travel)"));
  Serial.println(F("  m       -> micro test (tiny open/close)"));
  Serial.println(F("  r       -> ramp sweep (bigger motion)"));
  Serial.println(F("  o       -> move to OPEN"));
  Serial.println(F("  c       -> move to CLOSED"));
  Serial.println(F("  p       -> print all values"));
  Serial.print(F("Selected -> ")); Serial.print(selected);
  Serial.print(" : "); Serial.println(name[selected]);
}

void loop() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.startsWith("s ")) {
    int idx = cmd.substring(2).toInt();
    if (idx >= 0 && idx < 6) {
      selected = idx;
      Serial.print("Selected -> "); Serial.print(selected);
      Serial.print(" : "); Serial.println(name[selected]);
      Serial.print("  center="); Serial.print(centerPulse[selected]);
      Serial.print(" travel="); Serial.print(travelPulse[selected]);
      Serial.print(" direction="); Serial.println(direction[selected]);
    } else Serial.println("Index out of range (0..5)");
    return;
  }

  if (cmd == "+") {
    centerPulse[selected] = constrain(centerPulse[selected] + 1, 120, 600);
    pwm.setPWM(ch[selected], 0, centerPulse[selected]);
    return;
  }

  if (cmd == "-") {
    centerPulse[selected] = constrain(centerPulse[selected] - 1, 120, 600);
    pwm.setPWM(ch[selected], 0, centerPulse[selected]);
    return;
  }

  if (cmd == ">") {
    travelPulse[selected]++;
    return;
  }

  if (cmd == "<") {
    travelPulse[selected] = max(1, travelPulse[selected]-1);
    return;
  }

  if (cmd.startsWith("setc ")) {
    int v = cmd.substring(5).toInt();
    centerPulse[selected] = constrain(v, 120, 600);
    pwm.setPWM(ch[selected], 0, centerPulse[selected]);
    return;
  }

  if (cmd.startsWith("sett ")) {
    int v = cmd.substring(5).toInt();
    travelPulse[selected] = max(1, v);
    return;
  }

  if (cmd == "dir +") {
    direction[selected] = +1;
    Serial.print("Direction for "); Serial.print(name[selected]); Serial.println(" set to +1");
    return;
  }

  if (cmd == "dir -") {
    direction[selected] = -1;
    Serial.print("Direction for "); Serial.print(name[selected]); Serial.println(" set to -1");
    return;
  }

  if (cmd == "m") {
    microNudge(selected);
    return;
  }

  if (cmd == "r") {
    rampSweep(selected, 3);
    return;
  }

  if (cmd == "o") {
    int pulse = centerPulse[selected] + direction[selected] * travelPulse[selected];
    pwm.setPWM(ch[selected], 0, pulse);
    Serial.print("OPEN "); Serial.print(name[selected]); Serial.print(" -> "); Serial.println(pulse);
    return;
  }

  if (cmd == "c") {
    pwm.setPWM(ch[selected], 0, centerPulse[selected]);
    Serial.print("CLOSE "); Serial.println(name[selected]);
    return;
  }

  if (cmd == "p") {
    Serial.println("=== Current values ===");
    for (int i = 0; i < 6; i++) {
      Serial.print(i); Serial.print(": "); Serial.print(name[i]);
      Serial.print(" ch="); Serial.print(ch[i]);
      Serial.print(" center="); Serial.print(centerPulse[i]);
      Serial.print(" travel="); Serial.print(travelPulse[i]);
      Serial.print(" dir="); Serial.println(direction[i]);
    }
    Serial.println("======================");
    return;
  }

  Serial.println("Unknown command. Type p for status.");
}

// --- helpers ---

void microNudge(int idx) {
  int c = centerPulse[idx];
  int t = travelPulse[idx];
  int d = direction[idx];
  int open = constrain(c + d * t, 120, 600);
  pwm.setPWM(ch[idx], 0, open); delay(300);
  pwm.setPWM(ch[idx], 0, c);    delay(300);
  pwm.setPWM(ch[idx], 0, open); delay(200);
  pwm.setPWM(ch[idx], 0, c);
}

void rampSweep(int idx, int loops) {
  int c = centerPulse[idx];
  int t = travelPulse[idx] * 5;
  int d = direction[idx];
  for (int l=0; l<loops; l++) {
    pwm.setPWM(ch[idx], 0, c + d * t); delay(400);
    pwm.setPWM(ch[idx], 0, c - d * t); delay(400);
  }
  pwm.setPWM(ch[idx], 0, c);
}
