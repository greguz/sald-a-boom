# sald-a-boom

Yet another Sax-A-Boom clone project.

## Features

- For Sax-A-Boom lovers <3
- 8 banks
- 8 tracks per bank (or 9 in sax mode)
- 2 playback modes
  - Keyboard mode
  - Sax mode (air pressure sensor)
- Adjustable volume
- Rechargable battery (USB-C)
- Custom 3D printed parts
- Status LED

## Hardware

### Bill of Materials

- Bontempi Baby Saxophone (first random sax toy I found online)
- Raspberry Pi Pico (first version, non-wireless)
- PAM8302A (low voltage class-D mono amplifier)
- MAX4466 breakout board (electret microphone amplifier)
- Micro SD SPI board/module (without any voltage regulator)
- 2x 1Nxxxx diodes
- 1x potentiometer
- 1x 100 nF ceramic capacitor
- 1x 68 Ohm resistor
- 1x 1 KOhm resistor
- 1x 2.2 KOhm resistor
- Single cell Li-Po (3.7V nominal)
- Single cell (1S) USB C BMS board

### Default pinout

```
                  ┌─────┐
          ┌───────│     │───────┐
          │1      │     │     40│
          │2      └─────┘     39│ VIN (Lipo 1S)
      GND │3                  38│ GND
      CLK │4                  37│
     MOSI │5                  36│ 3V3
     MISO │6                  35│
      GND │7                  34│
Audio OUT │8                  33│ Pot IN
          │9      ┌─────┐     32│ MAX4466 IN
          │10     │     │     31│
          │11     │     │     30│
          │12     └─────┘     29│
      GND │13                 28│ GND
   BTN IN │14                 27│
   BTN IN │15                 26│
   BTN IN │16                 25│
   BTN IN │17  Raspberry Pi   24│
      GND │18      Pico       23│ GND
  BTN OUT │19     RP2040      22│
  BTN OUT │20                 21│
          └─────────────────────┘
```

### Blocks diagram

```
┌────────────────┐                 ┌──────────────┐
│ Buttons Matrix ├────────────────►│              │
└────────────────┘                 │              │
┌────────────┐     ┌─────────┐     │              │
│  Electret  ├────►│ MAX4466 ├────►│              │
│ Microphone │     └─────────┘     │ Raspberry Pi │
└────────────┘                     │     Pico     │
┌───────────────┐                  │              │
│ Potentiometer ├─────────────────►│              │
└───────────────┘                  │              │
┌──────────┐                       │              │
│ Micro SD ├──────────────────────►│              │
└──────────┘                       └─────────────┬┘
                                                 │
┌─────────┐     ┌──────────┐     ┌───────────┐   │
│ Speaker │◄────┤ PAM8302A │◄────┤ RC Filter │◄──┘
└─────────┘     └──────────┘     └───────────┘
```

## Software

### Environment

Install Raspberry Pi Pico C/C++ SDK or use the official VS Code extension.

Developed using the official VS Code extension on macOS (apple silicon).

### Project structure

- `ff` directory: [`FatFs` R0.16](https://elm-chan.org/fsw/ff/) FAT driver.
- `spi` directory: [Low level `FatFs` SPI driver](https://github.com/elehobica/pico_fatfs) for Raspberry Pi Pico.
- `core` directory: Application core modules.
- `sald-a-boom.c`: The main application executable.

## Special thanks

- https://gregchadwick.co.uk/blog/playing-with-the-pico-pt3/
- https://github.com/GregAC/pico-stuff/tree/main/pwm_audio
- https://elm-chan.org/fsw/ff/
- https://github.com/elehobica/pico_fatfs
- https://github.com/lewish/asciiflow
