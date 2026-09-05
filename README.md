# sald-a-boom

Yet another Sax-A-Boom clone project.

## Hardware

### Bill of Materials

- Bontempi Baby Saxophone (first sax toy I found online)
- Raspberry Pi Pico (first version, non-wireless)
- PAM8302A (low voltage class-D mono amplifier)
- MAX4466 breakout board (electret microphone)
- Micro SD SPI board/module (without any voltage regulator)
- 2x 1Nxxxx diodes
- 4x 220 Ohm resistors
- Li-Po single cell, 3.7V

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
