#pragma once

#include <Wire.h>
#include <DS3231.h>

#define UPDATE_DELAY 1000

DS3231 Clock;
byte Hour = 0;
byte Minute = 0;
bool Dots = false;
bool updating = false;
unsigned long lastTime = 0;

void showTime() {
  writeValue(
    (Hour / 10) % 10,
    Hour % 10, 
    (Minute / 10) % 10,
    Minute % 10
  );
}

void saveTime() {
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
}

void incHours(bool last) {
  updating = true;
  Hour++;
  if (Hour == 24) {
    Hour = 0;
  }
  showTime();
  if (last) {
    saveTime();
    updating = false;
  }
}

void incMinutes(bool last) {
  updating = true;
  Minute++;
  if (Minute == 60) {
    Minute = 0;
    Hour++;
    if (Hour == 24) {
      Hour = 0;
    }
  }
  showTime();
  if (last) {
    saveTime();
    updating = false;
  }
}

void incHour() {
  incHours(true);
}

void incMinute() {
  incMinutes(true);
}

void doSetup() {
  Wire.begin();

  Clock.setClockMode(false);

  button1.onSinglePress(onOff);
  button2.onSinglePress(incHour);
  button2.onSustain(incHours);
  button3.onSinglePress(incMinute);
  button3.onSustain(incMinutes);
}

void doLoop() {
  if (!updating && millis() - lastTime >= UPDATE_DELAY) {
    static bool h12, pmtime;
    Hour = Clock.getHour(h12, pmtime);
    Minute = Clock.getMinute();
  
    showTime();

    Dots = !Dots;
    if (Dots) {
      dotsOn();
    } else {
      dotsOff();
    }

    lastTime = millis();
  }
}
