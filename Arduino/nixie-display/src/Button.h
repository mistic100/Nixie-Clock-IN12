#pragma once

#define BTN_IDLE 0
#define BTN_SINGLE 1
#define BTN_SUSTAIN 2
#define BTN_RELEASE 3

#define BUTTON_DELAY 300
#define BUTTON_SUSTAIN_DELAY 1000
#define BUTTON_SUSTAIN_INTERVAL 50
#define BUTTON_ANALOG_THRESHOLD 200

class Button {
  
private:
  uint8_t pin;
  bool analog;

  unsigned long pressTime = 0;
  uint8_t state = BTN_IDLE;
  unsigned long sustainCount = 0;
  bool isDouble = false;

  void (*user_onSinglePress)(void);
  void (*user_onLongPress)(void);
  void (*user_onDoublePress)(void);
  void (*user_onSustain)(bool last, unsigned long ellapsed);

public:
  Button(uint8_t _pin, bool _analog = false) {
    pin = _pin;
    analog = _analog;
    if (!analog) {
      pinMode(pin, INPUT_PULLUP);
    }
  }

  void onSinglePress(void (*function)(void)) {
    user_onSinglePress = function;
  }

  void onLongPress(void (*function)(void)) {
    user_onLongPress = function;
  }

  void onDoublePress(void (*function)(void)) {
    user_onDoublePress = function;
  }

  void onSustain(void (*function)(bool last, unsigned long ellapsed)) {
    user_onSustain = function;
  }

  void handle() {
    bool pressed;
    if (analog) {
      pressed = analogRead(pin) <= BUTTON_ANALOG_THRESHOLD;
    } else {
      pressed = digitalRead(pin) == LOW;
    }

    if (pressed) {
      switch (state) {
        case BTN_RELEASE:
          isDouble = true;
        case BTN_IDLE:
          pressTime = millis();
          state = BTN_SINGLE;
          break;
          
        case BTN_SINGLE:
          if (millis() - pressTime > BUTTON_SUSTAIN_DELAY) {
            sustainCount = 1;
            if (user_onSustain) {
              user_onSustain(false, sustainCount * BUTTON_SUSTAIN_INTERVAL);
            }
            pressTime = millis();
            state = BTN_SUSTAIN;
          }
          break;
          
        case BTN_SUSTAIN:
          if (millis() - pressTime > BUTTON_SUSTAIN_INTERVAL) {
            sustainCount++;
            if (user_onSustain) {
              user_onSustain(false, sustainCount * BUTTON_SUSTAIN_INTERVAL);
            }
            pressTime = millis();
          }
          break;
      }
    } else {
      switch (state) {
        case BTN_SINGLE:
          if (millis() - pressTime < BUTTON_DELAY) {
            pressTime = millis();
            state = BTN_RELEASE;
          } else {
            if (user_onLongPress) {
              user_onLongPress();
            }
            state = BTN_IDLE;
            isDouble = false;
          }
          break;
          
        case BTN_SUSTAIN:
          sustainCount++;
          if (user_onSustain) {
            user_onSustain(true, sustainCount * BUTTON_SUSTAIN_INTERVAL);
          }
          state = BTN_IDLE;
          isDouble = false;
          break;
  
        case BTN_RELEASE:
          if (isDouble) {
            if (user_onDoublePress) {
              user_onDoublePress();
            }
            state = BTN_IDLE;
            isDouble = false;
          } else if (millis() - pressTime >= BUTTON_DELAY) {
            if (user_onSinglePress) {
              user_onSinglePress();
            }
            state = BTN_IDLE;
            isDouble = false;
          }
          break;
      }
    }
  }
  
};
