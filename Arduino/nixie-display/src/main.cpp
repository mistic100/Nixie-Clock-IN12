#include <Arduino.h>
#include <ShiftRegister74HC595.h>
#include <FastLED.h>
#include <DS3231.h>
#include <Wire.h>
#include <FlashStorage_SAMD.h>
#include "Button.h"

// automatic shutdown after X seconds
#define AUTO_OFF_DELAY 10 * 60

// define at which time the display stays on
const byte ALWAYS_ON_TIME[][3] = {
    {false, 0, 0}, // the first line must be 0:0
    {true, 9, 30},
    {false, 23, 30},
};

// invert the digits order, comment to disable
// #define INVERT

// enable seconds, comment to disable
// #define SIX_DIGITS

// number of NeoPixels
#define LEDS_NUM 4

// color order of NeoPixels
#define LEDS_ORDER GRB

// chipset of NeoPixels
#define LEDS_TYPE WS2812

// automatically run anti cathode poisonning after X seconds, comment to disable
#define ANTI_POISONING_DELAY 20 * 60

// END OF CONFIGURATION

// pins
#define DOTS_PIN 0
#define LEDS_PIN 6

#define SR_DS_PIN 3
#define SR_SHCP_PIN 2
#define SR_STCP_PIN 1

const byte SR_PINS[][4] = {
    {7, 5, 4, 6},
    {3, 1, 0, 2},
    {7 + 8, 5 + 8, 4 + 8, 6 + 8},
    {3 + 8, 1 + 8, 0 + 8, 2 + 8},
    {7 + 16, 5 + 16, 4 + 16, 6 + 16},
    {3 + 16, 1 + 16, 0 + 16, 2 + 16}};

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
enum LedsModes
{
  M_RAINBOW,
  M_RAINBOW_2,
  M_BLUE,
  M_CYAN,
  M_GREEN,
  M_ORANGE,
  M_RED,
  M_PURPLE,
  M_WHITE,
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
bool needSaveSettings = false;
bool needSaveTime = false;
bool alwaysOn = true;
unsigned long onTime = 0;
#ifdef ANTI_POISONING_DELAY
unsigned long antiPoisoningTime = 0;
#endif

void on();
void off();
void onOff();
void dotsOn();
void dotsOff();
void dotsOnOff();
bool isAlwaysOn();
void changeAlwaysOn();
void saveSettings();
void saveTime();
void incTime();
void getTime();
void showTime();
void oneArmedBandit();
void incSecond();
void incMinute();
void incHour();
void incSeconds(bool last, unsigned long ellapsed);
void incMinutes(bool last, unsigned long ellapsed);
void incHours(bool last, unsigned long ellapsed);
void ledsChangeBrightness(bool last, unsigned long);
void ledsNextMode();
void ledsRun();
void printTime();

void setup()
{
  sr.setAllLow();
  dotsOn();

  Serial.begin(9600);

  int signature;
  EEPROM.get(0, signature);
  if (signature == EEPROM_SIGNATURE)
  {
    alwaysOn = EEPROM.read(sizeof(EEPROM_SIGNATURE) + EEPROM_ALWAYS_ON) == 1;
    ledsMode = LedsModes(EEPROM.read(sizeof(EEPROM_SIGNATURE) + EEPROM_COLOR) % NUM_MODES);
    ledsBrightness = EEPROM.read(sizeof(EEPROM_SIGNATURE) + EEPROM_BRIGHTNESS);
  }

  FastLED.addLeds<LEDS_TYPE, LEDS_PIN, LEDS_ORDER>(leds, LEDS_NUM);
  FastLED.setCorrection(UncorrectedColor);
  FastLED.setBrightness(ledsBrightness);

  Wire.begin();
  Clock.setClockMode(false);

  button1.onSinglePress(onOff);
  button1.onLongPress(changeAlwaysOn);

  button2.onSinglePress(incHour);
  button2.onSustain(incHours);

  button3.onSinglePress(incMinute);
  button3.onSustain(incMinutes);

  button4.onSinglePress(ledsNextMode);
  button4.onSustain(ledsChangeBrightness);

  getTime();
  Serial.print("Time at init: ");
  printTime();

  on();
}

void loop()
{
  button1.handle();
  button2.handle();
  button3.handle();
  button4.handle();

  EVERY_N_SECONDS(1)
  {
    if (!updatingTime)
    {
      incTime();

      if (!isAlwaysOn() && millis() - onTime > AUTO_OFF_DELAY * 1000)
      {
        off();
      }

      if (isOn)
      {
        showTime();
        dotsOnOff();

#ifdef ANTI_POISONING_DELAY
        if (millis() - antiPoisoningTime > ANTI_POISONING_DELAY * 1000)
        {
          oneArmedBandit();
        }
#endif
      }
    }
  }

  EVERY_N_SECONDS(10)
  {
    if (needSaveSettings)
    {
      saveSettings();
    }
    if (needSaveTime && !updatingTime)
    {
      saveTime();
    }
  }

  EVERY_N_MINUTES(10)
  {
    if (!updatingTime && !needSaveTime)
    {
      getTime();
    }
  }

  EVERY_N_MILLIS(50)
  {
    if (isOn)
    {
      ledsRun();
      FastLED.setBrightness(ledsBrightness);
    }
  }

  if (isOn)
  {
    FastLED.show();
  }
}

/**
 * Enable display
 */
void on()
{
  if (!isOn)
  {
    Serial.println(F("ON"));
    isOn = true;
    showTime();
  }
  onTime = millis();
}

/**
 * Disable display
 */
void off()
{
  if (isOn)
  {
    Serial.println(F("OFF"));
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
void onOff()
{
  if (!isOn)
  {
    on();
  }
  else
  {
    off();
  }
}

/**
 * Switch between always on and auto off
 */
void changeAlwaysOn()
{
  on();

  alwaysOn = !alwaysOn;
  needSaveSettings = true;

  Serial.print("Always ON: ");
  Serial.println(alwaysOn ? "yes" : "no");
}

/**
 * Returns the always on status depending on current time
 */
bool isAlwaysOn()
{
  if (alwaysOn)
  {
    return true;
  }

  int n = sizeof(ALWAYS_ON_TIME) / sizeof(ALWAYS_ON_TIME[0]);
  bool res;

  for (int i = 0; i < n; i++)
  {
    if (Hour >= ALWAYS_ON_TIME[i][1] && Minute >= ALWAYS_ON_TIME[i][2])
    {
      res = ALWAYS_ON_TIME[i][0];
    }
    else
    {
      break;
    }
  }

  return res;
}

/**
 * Enable the dots
 */
void dotsOn()
{
  if (!Dots)
  {
    Dots = true;
    digitalWrite(DOTS_PIN, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

/**
 * Disable the dots
 */
void dotsOff()
{
  if (Dots)
  {
    Dots = false;
    digitalWrite(DOTS_PIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

/**
 * Disable or enable dots
 */
void dotsOnOff()
{
  if (!Dots)
  {
    dotsOn();
  }
  else
  {
    dotsOff();
  }
}

/**
 * Write the value of a digit
 */
void writeDigit(byte digit, uint8_t value)
{
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
 * Write values to all four digits
 */
void writeValue(
    uint8_t digit1,
    uint8_t digit2,
    uint8_t digit3,
    uint8_t digit4,
    uint8_t digit5,
    uint8_t digit6)
{
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
 * Display the current time
 */
void showTime()
{
  writeValue(
      (Hour / 10) % 10,
      Hour % 10,
      (Minute / 10) % 10,
      Minute % 10,
      (Second / 10) % 10,
      Second % 10);
}

/**
 * Increment local time by 1 second
 */
void incTime()
{
  Second++;
  if (Second == 60)
  {
    Second = 0;
    if (!updatingTime)
    {
      Minute++;
      if (Minute == 60)
      {
        Minute = 0;
        Hour++;
        if (Hour == 24)
        {
          Hour = 0;
        }
      }
    }
  }
}

/**
 * Get time from the clock
 */
void getTime()
{
  static bool h12, pmtime;
  Hour = Clock.getHour(h12, pmtime);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();
}

/**
 * Save time on the clock
 */
void saveTime()
{
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
  Clock.setSecond(Second);

  Serial.print("Update time: ");
  printTime();

  needSaveTime = false;
}

/**
 * Increase the hours
 */
void incHours(bool last, unsigned long)
{
  on();
  dotsOff();
  updatingTime = true;

  EVERY_N_MILLIS_I(timer, 500)
  {
    Hour++;
    if (Hour == 24)
    {
      Hour = 0;
    }
    showTime();
  }

  if (last)
  {
    needSaveTime = true;
    updatingTime = false;
  }
}

/**
 * Increase the minutes
 */
void incMinutes(bool last, unsigned long ellapsed)
{
  on();
  dotsOff();
  updatingTime = true;

  EVERY_N_MILLIS_I(timer, 500)
  {
    timer.setPeriod(ellapsed < 2000 ? 500 : 100);
    Minute++;
    if (Minute == 60)
    {
      Minute = 0;
    }
    showTime();
  }

  if (last)
  {
    timer.setPeriod(500);

    needSaveTime = true;
    updatingTime = false;
  }
}

/**
 * Increase the seconds
 */
void incSeconds(bool last, unsigned long ellapsed)
{
  on();
  dotsOff();
  updatingTime = true;

  EVERY_N_MILLIS_I(timer, 500)
  {
    timer.setPeriod(ellapsed < 2000 ? 500 : 100);
    Second++;
    if (Second == 60)
    {
      Second = 0;
    }
    showTime();
  }

  if (last)
  {
    timer.setPeriod(500);

    needSaveTime = true;
    updatingTime = false;
  }
}

/**
 * Increase the hours
 */
void incHour()
{
  on();

  Hour++;
  if (Hour == 24)
  {
    Hour = 0;
  }
  showTime();

  needSaveTime = true;
}

/**
 * Increase the minutes
 */
void incMinute()
{
  on();

  Minute++;
  if (Minute == 60)
  {
    Minute = 0;
  }
  showTime();

  needSaveTime = true;
}

/**
 * Increase the seconds
 */
void incSecond()
{
  on();

  Second++;
  if (Second == 60)
  {
    Second = 0;
  }
  showTime();

  needSaveTime = true;
}

/**
 * Cycle through all digits
 */
#ifdef ANTI_POISONING_DELAY
void oneArmedBandit()
{
  Serial.println("Cathode anti-poisoning");

  for (uint8_t i = 0; i < 100; i++)
  {
    writeValue(i % 10, i % 10, i % 10, i % 10, i % 10, i % 10);
    delay(10);
  }

  antiPoisoningTime = millis();

  showTime();
}
#endif

/**
 * Display the current leds palette
 */
void showPalette(uint8_t scale = 255)
{
  for (uint8_t i = 0; i < LEDS_NUM; i++)
  {
    uint8_t index = 1.0 * scale * i / (LEDS_NUM - 1);
    leds[i] = ColorFromPalette(ledsPalette, index + ledsPaletteIndex);
  }
}

/**
 * Execute leds animation
 */
void ledsRun()
{
  switch (ledsMode)
  {
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
  case M_WHITE:
    fill_solid(leds, LEDS_NUM, CRGB::White);
    break;
  }
}

/**
 * Change leds mode
 */
void ledsNextMode()
{
  on();

  ledsMode = LedsModes((ledsMode + 1) % NUM_MODES);
  ledsPaletteIndex = 0;
  needSaveSettings = true;

  Serial.print("LED mode: ");
  Serial.println(ledsMode);
}

/**
 * Change leds brightness
 */
void ledsChangeBrightness(bool last, unsigned long)
{
  on();

  if (ledsBrightnessDir)
  {
    ledsBrightness = qadd8(ledsBrightness, 16);
  }
  else
  {
    ledsBrightness = qsub8(ledsBrightness, 16);
  }
  FastLED.setBrightness(ledsBrightness);

  if (last)
  {
    ledsBrightnessDir = !ledsBrightnessDir;
    needSaveSettings = true;

    Serial.print("LED brightness: ");
    Serial.println(ledsBrightness);
  }
}

/**
 * Store settings
 */
void saveSettings()
{
  EEPROM.put(0, EEPROM_SIGNATURE);
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_ALWAYS_ON, alwaysOn ? 1 : 0);
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_COLOR, ledsMode);
  EEPROM.put(sizeof(EEPROM_SIGNATURE) + EEPROM_BRIGHTNESS, ledsBrightness);
  EEPROM.commit();

  Serial.println("Save settings");

  needSaveSettings = false;
}

/**
 * Prints the time to Serial
 */
void printTime()
{
  Serial.print(Hour);
  Serial.print(":");
  Serial.print(Minute);
  Serial.println();
}
