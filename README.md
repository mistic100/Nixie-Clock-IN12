# Nixie Clock 2

([Check my first clock](https://github.com/mistic100/Nixie-Clock))

This is a Nixie Clock design using IN-12 soviet neon tubes and an Arduino connected to a RTC clock. It also has a NeoPixels output.

The Arduino code has three modes :
- Clock : queries the current time on the RTC module
- Display : receive raw value from I2C (this was used as a score counter for my [robotic association](https://github.com/ARIG-Robotique)
- Test : manual change each digit


## Controls

There are four connectors to wire push buttons.

### Clock

- Button 1 : on/off
- Button 2 : change hours
- Button 3 : change minutes
- Button 4 : unused

### Display

- Button 1 : on/off
- Button 2 : random value
- Button 3 : unused
- Button 4 : unused

### Test

- Button 1 : change digit 1
- Button 2 : change digit 2
- Button 3 : change digit 3
- Button 4 : change digit 4


## NeoPixels

TODO


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

### Misc
| part | qty. | notes |
|--|--|--|
| 100µF capacitor | 1 | power filtering |
| IC terminal block | 1 |
| 2x4 2.54mm male connector | 1 | Buttons |
| 1x3 2.54mm male connector | 1 | NeoPixels strip |
| 1x2 2.54mm male connector + jumper | 1 | disconnects the microcontroller 5V input |

### 3D Printed
| part | qty. | notes |
|--|--|--|
| INS-1 holder | 1 |

## License

The Arduino code, the Fritzing design and SolidWorks files are distributed under the Creative Commons 3.0 BY-SA license.
