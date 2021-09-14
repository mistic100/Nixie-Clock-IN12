# Nixie Clock 2

([Check my first clock](https://github.com/mistic100/Nixie-Clock))

This is a Nixie Clock design using IN-12 soviet neon tubes and an Arduino connected to a RTC clock. It also has a NeoPixels output. A basic 3D printable "air case" is provided.

![](https://galerie.strangeplanet.fr/_data/i/upload/2021/09/05/20210905122303-39b00a0b-me.jpg)

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


## Parts list

### Tubes
| part | qty. | notes |
|--|--|--|
| IN-12 tubes | 4 |
| INS-1 indicator | 2 |
| IN-12 sockets pins | 48 |

### Tubes power
| part | qty. | notes |
|--|--|--|
| NCH8200HV power supply | 1 | [store](https://omnixie.com/products/nch8200hv-nixie-hv-power-module) - I use a clone with different pins location |
| 15kOhm resistor | 4 | IN-12 current limit |
| 270kOhm resistor | 2 | INS-1 current limit |

### Drivers
| part | qty. | notes |
|--|--|--|
| K155ID1 tube driver | 4 |
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
| 1x3 2.54mm male connector | 1 | NeoPixels strip |

### For "air case"
| part | qty. | notes |
|--|--|--|
| Micro-USB breakout board | 1 |
| 5mm push button | 2 |
| 12mm push button | 1 |
| Dupont cables | |
| hv-cover | 1 | 3d printed |
| indic-holder | 1 | 3d printed |
| leg-left | 1 | 3d printed |
| leg-right | 1 | 3d printed |
| plate-left | 1 | 3d printed |
| plate-right | 1 | 3d printed |


## License

The Arduino code, the Fritzing design and SolidWorks files are distributed under the Creative Commons 3.0 BY-SA license.
