#Testing Wireless modules
## Hardware
2x XL1276-D01 + Adafruit ProTrinket 3V
100nF 1206-case soldered on VCC-GND pins of the XL1276-D01
| XL1276-D01 | ProTrinket 3V |
| ----------:|--------------:|
| NSS | 10 |
| MOSI | 11 |
| MISO | 12 |
| SCK | 13 |
| REST | 9 |
| DIO0 | 3 |
| VCC | 3V |
| GND | G |

## Firmware
[Adafruit](https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/using-the-rfm-9x-radio) has a good example on how to use this library.
