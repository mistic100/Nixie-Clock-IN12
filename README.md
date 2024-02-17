# Nixie Clock 2

([Check my first clock](https://github.com/mistic100/Nixie-Clock))

This is a Nixie Clock design using IN-12 soviet neon tubes and an Arduino connected to a RTC clock. Two different cases are provided.

It has a NeoPixels port and locations for SMD WS2812 LEDs (not tested because I added them after building my own clock).

![](https://galerie.strangeplanet.fr/_data/i/upload/2021/09/05/20210905122303-39b00a0b-me.jpg)

![](https://galerie.strangeplanet.fr/_data/i/upload/2021/10/05/20211005212903-25f66541-me.jpg)

[Pictures on my website](https://galerie.strangeplanet.fr/index.php?/category/219)


## Controls

### On/off (button 1)

The clock automatically shuts down after 15s.

- Short press : on/off
- Double press : switch to always on mode

### Set time (buttons 2 & 3)

- Short press : increment the value
- Maintain press : increment the value faster

### NeoPixels (button 4)

- Short press : change color/effect
- Maintain press : change brightness


## Code

### Librairies

- ShiftRegister74HC595
- FastLED
- DS3231
- FlashStorage_SAMD

### Configuration

- `AUTO_OFF_DELAY` automatic shutdown after X milliseconds, comment to disable
- `INVERT` invert the digits (used for the full-case)
- `SIX_DIGITS` enable seconds (requires extension board)
- `LEDS_NUM` number of NeoPixels
- `LEDS_TYPE` color order of NeoPixels


## Parts list

### Tubes
| part | qty. | notes |
|--|--|--|
| IN-12 tubes | 4 | [store](https://tubes-store.com/product_info.php?products_id=38) |
| INS-1 indicator | 2 | [store](https://tubes-store.com/product_info.php?products_id=1323) |
| IN-12 sockets pins | 48 | [store](https://aliexpress.com/item/4001135699549.html) |

### Tubes power
| part | qty. | notes |
|--|--|--|
| NCH8200HV power supply | 1 | [store](https://omnixie.com/products/nch8200hv-nixie-hv-power-module) - I use a clone with different pins location |
| 15kOhm resistor | 4 | IN-12 current limit |
| 270kOhm resistor | 2 | INS-1 current limit |

### Drivers
| part | qty. | notes |
|--|--|--|
| K155ID1 tube driver | 4 | [store](https://tubes-store.com/product_info.php?products_id=46) |
| 74HC595 shift register | 2 |
| 100nF capacitor | 4 | IC power filtering |
| MPSA42 transistor | 1 | INS-1 control |
| 33kOhm resistor | 1 | transitor base |

### Controller
| part | qty. | notes |
|--|--|--|
| Seeeduino Xiao | 1 | [store](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html) |
| DS3231 RTC module | 1 |
| 1N4148 diode | 1 |

### Misc
| part | qty. | notes |
|--|--|--|
| 100µF capacitor | 1 | power filtering |
| IC terminal block | 1 |
| 2x4 2.54mm male connector | 1 | Buttons |
| 1x3 2.54mm male connector | 1 | External NeoPixels |


## "Air" case

This a simple stand which leaves the circuit exposed. It has no LEDs button.

### Parts list
| part | qty. | notes |
|--|--|--|
| SMD WS2812 LEDs | 4 | NeoPixels |
| Micro-USB breakout board | 1 |
| 5mm push button | 2 |
| 12mm push button | 1 |
| M3 screw + nut + washer | 4 |
| Dupont cables | |

### 3D printed parts
| part | qty. | notes |
|--|--|--|
| air-case/leg-left | 1 |
| air-case/leg-right | 1 |
| air-case/plate-left | 1 |
| air-case/plate-right | 1 |
| air-case/hv-cover | 1 |
| air-case/indic-holder | 1 |


## Desk case

This is a fully closed case. The tubes sockets are not soldered to the board but integrated in the structure and connected with a lot of wires.

### Parts list
| part | qty. | notes |
|--|--|--|
| ADA1938 LEDs | 4 | NeoPixels |
| 5.5mm DC jack | 1 |
| 5mm push button | 3 |
| 12mm push button | 1 |
| 1x3 2.54mm male+female connector | 2 | remove the central pin, used to make the INS-1 connector |
| M3 screw + nut + washer | 8 |
| M3 screw + insert | 2 |
| lot of wires | |

### 3D printed parts
| part | qty. | notes |
|--|--|--|
| full-case/box | 1 |
| full-case/beams | 1 |
| full-case/back-panel | 1 |
| full-case/front-panel | 1 |
| full-case/tubes-support | 1 |
| full-case/back-buttons-holder | 1 |
| full-case/top-button | 1 |
| full-case/top-button-holder | 1 |


## Extension board

There is a "nixie-display_extension.fzz" PCB design which allows to add two additional tubes. It will require to "hack" into the main board to get the necessary data lines.

- LED : output of the last Neopixel
- DS : output (Q7') of the last shift register
- STCP : any of the STCP pins of the shift registers, or pin D1 of the Xiao
- SHCP : any of the SHCP pins of the shift registers, or pin D2 of the Xiao
- 5V/GND : from the main terminal
- 170V : output of the NCH8200HV


## License

The Arduino code, the Fritzing design and SolidWorks files are distributed under the Creative Commons 3.0 BY-SA license.
