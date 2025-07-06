# sald-a-boom

- Bontempi Baby Saxophone (first sax toy I found online)
- Arduino MKR ZERO (low power, SD card slot, enough pins, [ArduinoSound](https://docs.arduino.cc/libraries/arduinosound/) lib available)
- MKR Proto Shield
  - 2x 1Nxxxx diodes
  - 4x 220 Ohm resistors
- MAX98357 breakout board (I2S audio aplifier)
- MAX4466 breakout board (electret microphone)
- 10 KOhm potentiometer
- LiPo Battery

## Connections

- MAX4466 breakout board
  - GND: GND
  - VIN: VCC (+3V3)
- MAX08357 breakout board:
  - GND: GND
  - VIN: VCC (+3V3)
  - LRC: 3
  - BCLK: 2
  - DIN: A6

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
