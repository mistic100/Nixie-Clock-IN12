#include <ShiftRegister74HC595.h>
#include <FastLED.h>
#include <DS3231.h>
#include <Wire.h>
#include <FlashStorage_SAMD.h>
#include "Button.h"

// automatic shutdown after X seconds, comment to disable
#define AUTO_OFF_DELAY 30

// invert the digits (used for the full-case), comment to disable
//#define INVERT

// enable seconds, comment to disable
//#define SIX_DIGITS

// number of NeoPixels
#define LEDS_NUM 4

// color order of NeoPixels
#define LEDS_TYPE GRB

// automatically run anti cathode poisonning after X seconds, comment to disable
#define ANTI_POISONING_DELAY 1200

// END OF CONFIGURATION

// pins
#define DOTS_PIN 0
#define LEDS_PIN 6

#define SR_DS_PIN 3
#define SR_SHCP_PIN 2
#define SR_STCP_PIN 1

const byte SR_PINS[][4] = {
  { 7, 5, 4, 6 },
  { 3, 1, 0, 2 },
  { 7+8, 5+8, 4+8, 6+8 },
  { 3+8, 1+8, 0+8, 2+8 },
  { 7+16, 5+8, 4+8, 6+8 },
  { 3+16, 1+16, 0+16, 2+16 }
};

#ifdef SIX_DIGITS
#define SR_SIZE 3
#else
#define SR_SIZE 2
#endif

// devices
ShiftRegister74HC595<SR_SIZE> sr(SR_DS_PIN, SR_SHCP_PIN, SR_STCP_PIN);

Button button1(10);
Button button2(9);
Button button3(8);
Button button4(7);

// eeprom configuration
#define EEPROM_ALWAYS_ON 0
#define EEPROM_COLOR 1
#define EEPROM_BRIGHTNESS 2
const int EEPROM_SIGNATURE = 0xBEEFDEED;

// time
DS3231 Clock;
byte Hour = 0;
byte Minute = 0;
byte Second = 0;
bool Dots = false;
bool updatingTime = false;

// leds
enum LedsModes {
  M_RAINBOW,
  M_RAINBOW_2,
  M_BLUE,
  M_CYAN,
  M_GREEN,
  M_ORANGE,
  M_RED,
  M_PURPLE,
  NUM_MODES
};

CRGB leds[LEDS_NUM];
enum LedsModes ledsMode = M_RAINBOW;
uint8_t ledsBrightness = 255;
CRGBPalette16 ledsPalette = RainbowColors_p;
uint8_t ledsPaletteIndex = 0;
bool ledsBrightnessDir = false;

// state
bool isOn = true;
bool needSave = false;
#ifdef ANTI_POISONING_DELAY
unsigned antiPoisoningTime =0;
#endif
#ifdef AUTO_OFF_DELAY
bool alwaysOn = false;
unsigned long onTime = millis();
#endif

void setup() {
  pinMode(DOTS_PIN, OUTPUT);
  
  sr.setAllLow();
  dotsOn();

  int signature;
  EEPROM.get(0, signature);
  if (signature == EEPROM_SIGNATURE) {
    #ifdef AUTO_OFF_DELAY
    alwaysOn = EEPROM.read(sizeof(EEPROM_SIGNATURE) + EEPROM_ALWAYS_ON) == 1;
    #endif
    ledsMode = LedsModes(EEPROM.read(sizeof(EEPROM_SIGNATURE) +EEPROM_COLOR) % NUM_MODES);
    ledsBrightness = EEPROM.read(sizeof(EEPROM_SIGNATURE) + EEPROM_BRIGHTNESS);
  }

  Serial.begin(9600);

  FastLED.addLeds<WS2812, LEDS_PIN, LEDS_TYPE>(leds, LEDS_NUM);
  FastLED.setCorrection(TypicalLEDStrip);

  Wire.begin();
  Clock.setClockMode(false);

  button1.onSinglePress(onOff);
  #ifdef AUTO_OFF_DELAY
  button1.onDoublePress(changeAlwaysOn);
  #endif

  button2.onSinglePress(incHour);
  button2.onSustain(incHours);
  
  button3.onSinglePress(incMinute);
  button3.onSustain(incMinutes);
  
  button4.onSinglePress(ledsNextMode);
  button4.onSustain(ledsChangeBrightness);

  #ifdef ANTI_POISONING_DELAY
  oneArmedBandit();
  #endif

  getTime();
  showTime();
}

void loop() {
  EVERY_N_MILLIS(50) {
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
    #ifdef AUTO_OFF_DELAY
    if (!updatingTime && !alwaysOn && millis() - onTime > AUTO_OFF_DELAY * 1000) {
      off();
    }
    #endif

    if (!updatingTime) {
      getTime();
    }
    
    if (!updatingTime && isOn) {
      showTime();
      dotsOnOff();

      #ifdef ANTI_POISONING_DELAY
      antiPoisoningTime++;
      if (antiPoisoningTime >= ANTI_POISONING_DELAY) {
        oneArmedBandit();
        antiPoisoningTime = 0;
      }
      #endif
    }
  }

  EVERY_N_MILLIS(10000) {
    if (needSave) {
      saveSettings();
      needSave = false;
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
  #ifdef AUTO_OFF_DELAY
  onTime = millis();
  #endif
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
#ifdef AUTO_OFF_DELAY
void changeAlwaysOn() {
  alwaysOn = !alwaysOn;
  on();
  needSave = true;
}
#endif

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
void writeValue(
    uint8_t digit1, 
    uint8_t digit2, 
    uint8_t digit3, 
    uint8_t digit4, 
    uint8_t digit5, 
    uint8_t digit6
) {
  Serial.print(F("Write "));
  Serial.print(digit1);
  Serial.print(digit2);
  Serial.print(F(":"));
  Serial.print(digit3);
  Serial.print(digit4);
  #ifdef SIX_DIGITS
  Serial.print(F(":"));
  Serial.print(digit5);
  Serial.print(digit6);
  #endif
  Serial.println();
  writeDigit(0, digit1);
  writeDigit(1, digit2);
  writeDigit(2, digit3);
  writeDigit(3, digit4);
  #ifdef SIX_DIGITS
  writeDigit(4, digit5);
  writeDigit(5, digit6);
  #endif
  sr.updateRegisters();
}

/**
 * Write the value of a digit
 */
void writeDigit(byte digit, uint8_t value) {
  #ifdef INVERT
    #ifdef SIX_DIGITS
    digit = 5 - digit;
    #else
    digit = 3 - digit;
    #endif
  #endif
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
    Minute % 10, 
    (Second / 10) % 10,
    Second % 10
  );
}

/**
 * Get time from the clock
 */
void getTime() {
  static bool h12, pmtime;
  Hour = Clock.getHour(h12, pmtime);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();
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
  on();

  Hour++;
  if (Hour == 24) {
    Hour = 0;
  }

  saveTime();
}

/** 
 * Increase the minutes
 */
void incMinute() {
  on();

  Minute++;
  if (Minute == 60) {
    Minute = 0;
    Hour++;
    if (Hour == 24) {
      Hour = 0;
    }
  }

  saveTime();
}

/**
 * Cycle through all digits
 */
#ifdef ANTI_POISONING_DELAY
void oneArmedBandit() {
  for (uint8_t i = 0; i < 100; i++) {
    writeValue(i%10, i%10, i%10, i%10, i%10, i%10);
    delay(10);
  }
}
#endif

/**
 * Display the current leds palette
 */
void showPalette(uint8_t scale = 255) {
  for (uint8_t i = 0; i < LEDS_NUM; i++) {
    uint8_t index = 1.0 * scale * i / (LEDS_NUM - 1);
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
      showPalette(64);
      break;

    case M_RAINBOW_2:
      ledsPaletteIndex += 1;
      showPalette(1);
      break;
      
    case M_BLUE:
      fill_solid(leds, LEDS_NUM, CRGB::Blue);
      break;
    case M_CYAN:
      fill_solid(leds, LEDS_NUM, CRGB::Cyan);
      break;
    case M_GREEN:
      fill_solid(leds, LEDS_NUM, CRGB::Green);
      break;
    case M_ORANGE:
      fill_solid(leds, LEDS_NUM, CRGB::OrangeRed);
      break;
    case M_RED:
      fill_solid(leds, LEDS_NUM, CRGB::Red);
      break;
    case M_PURPLE:
      fill_solid(leds, LEDS_NUM, CRGB::Purple);
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
  needSave = true;
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
    needSave = true;
  }
}

/**
 * Store settings
 */
void saveSettings() {
  EEPROM.put(0, EEPROM_SIGNATURE);
  #ifdef AUTO_OFF_DELAY
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_ALWAYS_ON, alwaysOn ? 1 : 0);
  #endif
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_COLOR, ledsMode);
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_BRIGHTNESS, ledsBrightness);
  EEPROM.commit();
}
