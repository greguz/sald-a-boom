# sald-a-boom

Yet another Sax-A-Boom clone project.

> Why? To add more features `:]`

## Hardware

### Bill of Materials

- Bontempi Baby Saxophone (first sax toy I found online)
- Arduino MKR ZERO (low power, SD card slot, enough pins, [ArduinoSound](https://docs.arduino.cc/libraries/arduinosound/) lib available)
- MKR Proto Shield
- MAX98357 breakout board (I2S audio aplifier)
- MAX4466 breakout board (electret microphone)
- 2x 1Nxxxx diodes
- 4x 220 Ohm resistors
- 10 KOhm potentiometer
- Li-Po single cell, 3.7V ([700mAh minimum](https://store.arduino.cc/products/arduino-mkr-zero-i2s-bus-sd-for-sound-music-digital-audio-data))

### Connections

- MAX4466 breakout board
  - GND: GND
  - VIN: VCC (+3V3)
- MAX08357 breakout board:
  - GND: GND
  - VIN: VCC (+3V3)
  - LRC: 3
  - BCLK: 2
  - DIN: A6

## Software

### Variables

- `PIN_MIC`:          MAX4466 board _OUT_ pin, default is `A1`
- `PIN_VOLUME`:       Potentiometer _OUT_ pin (middle one), default is `A2`
- `MIC_THRESHOLD`:   `2048`    // 0 to  4095 20Hz delta, 50% (2047 is silence)
- `POLL_TIMEOUT`:    `50`      // Milliseconds between buttons polling
- `PRESSURE_WINDOW`: `100` // Milliseconds
- `CONFIG_TIMEOUT`:  `3000`    // Milliseconds before CONFIG mode
- `PIN_COL_0`:       `6`       // Buttons grid digital outputs
- `PIN_COL_1`:       `7`
- `PIN_ROW_0`:       `11`      // Buttons grid digital inputs (pullup)
- `PIN_ROW_1`:       `10`
- `PIN_ROW_2`:       `9`
- `PIN_ROW_3`:       `8`

## Usage

### Config mode

1. Keyboard mode (push and play)
2. Sax mode (push with air pressure to play)
3. Loop mode (loop or pause)
4. x
5. Bank 0 (from `TRACK_01.WAV` to `TRACK_08.WAV`)
6. Bank 1 (from `TRACK_11.WAV` to `TRACK_18.WAV`)
7. Bank 2 (from `TRACK_21.WAV` to `TRACK_28.WAV`)
8. Bank 3 (from `TRACK_31.WAV` to `TRACK_38.WAV`)

## Remove battery holder

https://www.tinkercad.com/things/8PptFBEXruP-sald-a-boom
