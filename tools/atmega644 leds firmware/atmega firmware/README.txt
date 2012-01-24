Firmware for an Atmel Atmega644 µPC featuring
 - soft pwm for up to 16 pwm channels
 - curtain motor h-bridge control (direction+enable output) with support for 2 sensors: rotary+end
 - ethernet udp control via enc 28j60

Build:
- Use make to build the firmware (gcc-avr + avr-libc needed)

Linux Transfer Script:
- Use ./transfer to download the firmware to your ISP flashable Atmega644 via avrdude/usb
