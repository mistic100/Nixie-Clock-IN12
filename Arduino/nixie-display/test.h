#pragma once

uint8_t digit1 = 0;
uint8_t digit2 = 0;
uint8_t digit3 = 0;
uint8_t digit4 = 0;
bool changed = false;

void inc1() {
  digit1++;
  if (digit1 > 9) digit1 = 0;
  changed = true;
}
void inc2() {
  digit2++;
  if (digit2 > 9) digit2 = 0;
  changed = true;
}
void inc3() {
  digit3++;
  if (digit3 > 9) digit3 = 0;
  changed = true;
}
void inc4() {
  digit4++;
  if (digit4 > 9) digit4 = 0;
  changed = true;
}

void doSetup() {
  button1.onSinglePress(inc1);
  button2.onSinglePress(inc2);
  button3.onSinglePress(inc3);
  button4.onSinglePress(inc4);
}

void doLoop() {
  if (changed) {
    writeValue(digit1, digit2, digit3, digit4);
    changed = false;
  }
}
