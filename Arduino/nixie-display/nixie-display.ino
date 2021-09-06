#include <ShiftRegister74HC595.h>
#include <FastLED.h>
#include <DS3231.h>
#include <Wire.h>
#include "Button.h"

#define LEDS_NUM 7
#define AUTO_OFF_DELAY 15000

const byte SR_PINS[][4] = {
  { 7, 5, 4, 6 },
  { 3, 1, 0, 2 },
  { 7+8, 5+8, 4+8, 6+8 },
  { 3+8, 1+8, 0+8, 2+8 }
};

#define DOTS_PIN 0
#define LEDS_PIN 6

ShiftRegister74HC595<2> sr(3, 2, 1);

Button button1(10);
Button button2(9);
Button button3(8);
Button button4(7);

enum LedsModes {
  M_RAINBOW,
  M_RAINBOW_2,
  M_BLUE,
  M_GREEN,
  M_RED,
  NUM_MODES
};

// time
DS3231 Clock;
byte Hour = 0;
byte Minute = 0;
bool Dots = false;
bool updatingTime = false;

// leds
CRGB leds[LEDS_NUM];
enum LedsModes ledsMode = M_RAINBOW;
uint8_t ledsBrightness = 255;
CRGBPalette16 ledsPalette = RainbowColors_p;
uint8_t ledsPaletteIndex = 0;
bool ledsBrightnessDir = false;

bool isOn = true;
bool alwaysOn = false;
unsigned long onTime = millis();

void setup() {
  pinMode(DOTS_PIN, OUTPUT);
  
  sr.setAllLow();
  dotsOn();

  Serial.begin(9600);

  FastLED.addLeds<WS2812, LEDS_PIN, GRB>(leds, LEDS_NUM);
  FastLED.setCorrection(TypicalLEDStrip);

  Wire.begin();
  Clock.setClockMode(false);

  button1.onSinglePress(onOff);
  button1.onDoublePress(changeAlwaysOn);

  button2.onSinglePress(incHour);
  button2.onSustain(incHours);
  
  button3.onSinglePress(incMinute);
  button3.onSustain(incMinutes);
  
  button4.onSinglePress(ledsNextMode);
  button4.onSustain(ledsChangeBrightness);
}

void loop() {
  EVERY_N_MILLIS(10) {
    button1.handle();
    button2.handle();
    button3.handle();
    button4.handle();

    if (isOn) {
      ledsRun();
      FastLED.setBrightness(ledsBrightness);
      FastLED.show();
    }
  }

  EVERY_N_MILLIS(1000) {
    #if AUTO_OFF_DELAY > 0
    if (!updatingTime && !alwaysOn && millis() - onTime > AUTO_OFF_DELAY) {
      off();
    }
    #endif

    if (!updatingTime) {
      getTime();
    }
    
    if (!updatingTime && isOn) {
      showTime();
      dotsOnOff();
    }
  }
}

/**
 * Enable display
 */
void on() {
  if (!isOn) {
    Serial.println(F("on"));
    isOn = true;
    showTime();
  }
  onTime = millis();
}

/**
 * Disable display
 */
void off() {
  if (isOn) {
    Serial.println(F("off"));
    isOn = false;
  
    sr.setAllHigh();
    dotsOff();
    fill_solid(leds, LEDS_NUM, CRGB::Black);
    FastLED.show();
  }
}

/**
 * Disable or enable display
 */
void onOff() {
  if (!isOn) {
    on();
  } else {
    off();
  }
}

/**
 * Switch between always on and auto off
 */
void changeAlwaysOn() {
  #if AUTO_OFF_DELAY > 0
  alwaysOn = !alwaysOn;
  on();
  #endif
}

/**
 * Enable the dots
 */
void dotsOn() {
  if (!Dots) {
    Dots = true;
    digitalWrite(DOTS_PIN, HIGH);
  }
}

/**
 * Disable the dots
 */
void dotsOff() {
  if (Dots) {
    Dots = false;
    digitalWrite(DOTS_PIN, LOW);
  }
}

/**
 * Disable or enable dots
 */
void dotsOnOff() {
  if (!Dots) {
    dotsOn();
  } else {
    dotsOff();
  }
}

/**
 * Write values to all four digits
 */
void writeValue(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
  Serial.print(F("Write "));
  Serial.print(digit1);
  Serial.print(F(" "));
  Serial.print(digit2);
  Serial.print(F(" "));
  Serial.print(digit3);
  Serial.print(F(" "));
  Serial.print(digit4);
  Serial.println();
  writeDigit(0, digit1);
  writeDigit(1, digit2);
  writeDigit(2, digit3);
  writeDigit(3, digit4);
  sr.updateRegisters();
}

/**
 * Write the value of a digit
 */
void writeDigit(byte digit, uint8_t value) {
  sr.setNoUpdate(SR_PINS[digit][0], value & 0x01);
  sr.setNoUpdate(SR_PINS[digit][1], (value & 0x02) >> 1);
  sr.setNoUpdate(SR_PINS[digit][2], (value & 0x04) >> 2);
  sr.setNoUpdate(SR_PINS[digit][3], (value & 0x08) >> 3);
}

/**
 * Display the current time
 */
void showTime() {
  writeValue(
    (Hour / 10) % 10,
    Hour % 10, 
    (Minute / 10) % 10,
    Minute % 10
  );
}

/**
 * Get time from the clock
 */
void getTime() {
  static bool h12, pmtime;
  Hour = Clock.getHour(h12, pmtime);
  Minute = Clock.getMinute();
}

/**
 * Save time on the clock
 */
void saveTime() {
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
}

/** 
 * Increase the hours
 */
void incHours(bool last, unsigned long ellapsed) {
  on();
  dotsOff();
  updatingTime = true;
  
  EVERY_N_MILLIS_I(timer, 500) {
    timer.setPeriod(ellapsed < 2000 ? 500 : 100);
    Hour++;
    if (Hour == 24) {
      Hour = 0;
    }
    showTime();
  }
  
  if (last) {
    saveTime();
    updatingTime = false;
  }
}

/** 
 * Increase the minutes
 */
void incMinutes(bool last, unsigned long ellapsed) {
  on();
  dotsOff();
  updatingTime = true;
  
  EVERY_N_MILLIS_I(timer, 500) {
    timer.setPeriod(ellapsed < 2000 ? 500 : 100);
    Minute++;
    if (Minute == 60) {
      Minute = 0;
      Hour++;
      if (Hour == 24) {
        Hour = 0;
      }
    }
    showTime();
  }
  
  if (last) {
    saveTime();
    updatingTime = false;
  }
}

/** 
 * Increase the hours
 */
void incHour() {
  incHours(true, 0);
}

/** 
 * Increase the minutes
 */
void incMinute() {
  incMinutes(true, 0);
}

/**
 * Display the current leds palette
 */
void showPalette(uint8_t scale = 255) {
  for (uint8_t i = 0; i < LEDS_NUM; i++) {
    // 240 is 16x15, hi-bit is 15, low-bit is 0, this prevents ColorFromPalette to perform blending with the first color
    uint8_t index = min(1.0 * scale * i / (LEDS_NUM - 1), 240);
    leds[i] = ColorFromPalette(ledsPalette, index + ledsPaletteIndex);
  }
}

/**
 * Execute leds animation
 */
void ledsRun() {
  switch (ledsMode) {
    case M_RAINBOW:
      ledsPaletteIndex += 1;
      showPalette();
      break;

    case M_RAINBOW_2:
      ledsPaletteIndex += 1;
      showPalette(1);
      break;
      
    case M_BLUE:
      fill_solid(leds, LEDS_NUM, CRGB::Blue);
      break;
    case M_GREEN:
      fill_solid(leds, LEDS_NUM, CRGB::Green);
      break;
    case M_RED:
      fill_solid(leds, LEDS_NUM, CRGB::Red);
      break;
  }
}

/**
 * Change leds mode
 */
void ledsNextMode() {
  on();
  
  ledsMode = LedsModes((ledsMode + 1) % NUM_MODES);
  ledsPaletteIndex = 0;

  switch (ledsMode) {
    case M_RAINBOW:
    case M_RAINBOW_2:
      ledsPalette = RainbowColors_p;
      break;
  }
}

/**
 * Change leds brightness
 */
void ledsChangeBrightness(bool last, unsigned long) {
  on();
  
  if (ledsBrightnessDir) {
    ledsBrightness = qadd8(ledsBrightness, 16);
  } else {
    ledsBrightness = qsub8(ledsBrightness, 16);
  }
  if (last) {
    ledsBrightnessDir = !ledsBrightnessDir;
  }
}
