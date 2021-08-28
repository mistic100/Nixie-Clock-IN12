#pragma once

#include <Wire.h>

#define I2C_ADDRESS 0x2b
#define UPDATE_DELAY 200

unsigned long lastTime = 0;
unsigned int nextVal = 0;
unsigned int currentVal = 0;

void receiveEvent(int numBytes) {
  unsigned int newVal = 0;
  for (int i = 0; i < numBytes; i++) {
    newVal += Wire.read() << (numBytes * 8);
  }
  nextVal = newVal;
}

void showRandom() {
  nextVal = random(1024);
}

void doSetup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);

  button1.onSinglePress(onOff);
  button2.onSinglePress(showRandom);
}

void doLoop() {
  if (millis() - lastTime >= UPDATE_DELAY) {
    bool change = false;
    if (currentVal < nextVal) {
      currentVal++;
      change = true;
    }
    if (currentVal > nextVal) {
      currentVal--;
      change = true;
    }
  
    if (change) {
      writeValue(
        (currentVal / 1000) % 10, 
        (currentVal / 100) % 10, 
        (currentVal / 10) % 10,
        currentVal % 10
      );
    }
  
    lastTime = millis();
  }
}
