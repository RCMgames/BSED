# Byte Sized Encoder Decoder

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/joshua1024/library/byte-sized-encoder-decoder.svg)](https://registry.platformio.org/libraries/joshua1024/byte-sized-encoder-decoder)
[![arduino-library-badge](https://www.ardu-badge.com/badge/byte-sized-encoder-decoder.svg?)](https://www.ardu-badge.com/byte-sized-encoder-decoder)
[![Arduino Lint](https://github.com/RCMgames/BSED/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/RCMgames/BSED/actions/workflows/arduino-lint.yml)
[![doxygen](https://github.com/RCMgames/BSED/actions/workflows/doxygen.yml/badge.svg)](https://github.com/RCMgames/BSED/actions/workflows/doxygen.yml)

Reads 8 quadrature encoders and communicates over I2C.

https://github.com/RCMgames/BSED

* Reads 8 quadrature encoders
* 1.0x0.85 inches, 25.4x21.59 millimeters
* I2C (Qwiic Compatible)
* powered by an ATmega328PB
* hand solderable surface mount components

### links 
(this repository contains an Arduino library, firmware for the board, and hardware for the board)
* [Documentation for Library](https://rcmgames.github.io/BSED/class_byte_sized_encoder_decoder.html)
* [Hardware](https://github.com/RCMgames/BSED/tree/main/extras/hardware)
  * If you buy PCBs from PCBWay through this link PCBWay generously gives me a 10% commission without costing you anything
    * https://www.pcbway.com/project/shareproject/Byte_Sized_Encoder_Decoder_v1_0_5b76fc5d.html
* [Firmware](https://github.com/RCMgames/BSED/tree/main/extras/firmware)

### testing
Approximately 15000 encoder counts per second can be read from each encoder, but do your own testing before trusting this software.

See, and contribute to, this discussion on testing results: https://github.com/RCMgames/BSED/discussions/6

![front](https://github.com/RCMgames/BSED/blob/fd907d3367903d2f684a02541658e857a268ef72/extras/hardware/photos/P1039300.JPG)
![back](https://github.com/RCMgames/BSED/blob/fd907d3367903d2f684a02541658e857a268ef72/extras/hardware/photos/P1039304.JPG)

I've been working on [hardware and software for small wifi controlled robots](https://github.com/rcmgames) since 2020.

As I have designed increasingly small [circuit boards](https://github.com/rcmgames) for controlling servos and motors on robots, I started looking for a way to connect lots of quadrature encoders as inputs to a microcontroller. I was unable to find any existing Qwiic-compatible encoder reading boards that can handle multiple high-speed quadrature inputs. This board can help you control 8 motors with encoder feedback for precise control.

This version of firmware and library are compatible with version 1 of the hardware.

## Acknowledgements
* I would like to thank [PCBWay](https://www.pcbway.com/) for sponsoring prototyping runs of this project. PCBWay produces very nice boards, supports open source hardware, and gave me great support as I worked on this project. Special thanks to Liam!
