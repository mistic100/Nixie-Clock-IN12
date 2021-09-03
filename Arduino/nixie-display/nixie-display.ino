#include <ShiftRegister74HC595.h>
#include "Button.h"

#define MODE_CLOCK 1
#define MODE_COUNTER 2
#define MODE_TEST 0

#define MODE MODE_TEST

#define UPDATE_DELAY BUTTON_SUSTAIN_INTERVAL / 2

const byte SR_PINS[][4] = {
  { 7, 5, 4, 6 },
  { 3, 1, 0, 2 },
  { 7+8, 5+8, 4+8, 6+8 },
  { 3+8, 1+8, 0+8, 2+8 }
};

const byte DOTS_PIN = 0;

ShiftRegister74HC595<2> sr(3, 2, 1);

Button button1(10);
Button button2(9);
Button button3(8);
Button button4(7);

unsigned long lastUpdateTime = 0;
bool on = true;

void onOff() {
  on = !on;
  if (!on) {
    sr.setAllHigh();
    dotsOff();
  }
}

void dotsOn() {
  digitalWrite(DOTS_PIN, LOW);
}

void dotsOff() {
  digitalWrite(DOTS_PIN, HIGH);
}

void writeValue(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
  writeDigit(0, digit1);
  writeDigit(1, digit2);
  writeDigit(2, digit3);
  writeDigit(3, digit4);
  sr.updateRegisters();
}

void writeDigit(byte digit, uint8_t value) {
  Serial.print("Digit ");
  Serial.print(digit);
  Serial.print(" = ");
  Serial.print(value);

  Serial.print(" A:");
  Serial.print(value & 0x01);
  Serial.print(" B:");
  Serial.print((value & 0x02) >> 1);
  Serial.print(" C:");
  Serial.print((value & 0x04) >> 2);
  Serial.print(" D:");
  Serial.print((value & 0x08) >> 3);

  Serial.println();
  
  sr.setNoUpdate(SR_PINS[digit][0], value & 0x01);
  sr.setNoUpdate(SR_PINS[digit][1], (value & 0x02) >> 1);
  sr.setNoUpdate(SR_PINS[digit][2], (value & 0x04) >> 2);
  sr.setNoUpdate(SR_PINS[digit][3], (value & 0x08) >> 3);
}

#if MODE == MODE_CLOCK
#include "clock.h"
#elif MODE == MODE_COUNTER
#include "counter.h"
#else
#include "test.h"
#endif

void setup() {
  pinMode(DOTS_PIN, OUTPUT);
  
  sr.setAllHigh(); // disable all digits
  dotsOff();

  Serial.begin(9600);

  delay(1000);

  doSetup();
}

void loop() {
  if (millis() - lastUpdateTime >= UPDATE_DELAY) {
    button1.handle();
    button2.handle();
    button3.handle();
    button4.handle();

    if (on) {
      doLoop();
    }
    
    lastUpdateTime = millis();
  }
}
