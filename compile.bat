avr-gcc -mmcu=atmega640 -c main.c -o main.o
avr-gcc -mmcu=atmega640 -c command_handler.c -o command_handler.o
avr-gcc -mmcu=atmega640 -c pantilt.c -o pantilt.o
avr-gcc -mmcu=atmega640 -c analog.c -o analog.o
avr-gcc -mmcu=atmega640 -c usart.c -o usart.o
avr-gcc -mmcu=atmega640 -c array.c -o array.o

avr-gcc -mmcu=atmega640 -Wl,-u,vfprintf -lprintf_flt -lm main.o analog.o usart.o command_handler.o pantilt.o array.o -o gen.elf

avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature gen.elf BOB4_AngleOverlay.hex

avr-objcopy -O ihex -j .eeprom gen.elf EEPROM_DATA.hex

avr-size --mcu=atmega640 -C gen.elf

@echo off
del gen.elf
del /q /f *.o
pause