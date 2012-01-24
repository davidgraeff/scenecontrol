make clean && make && sudo avrdude -P usb -c usbasp -p m644 -U flash:w:firmware.hex
