avr-gcc -mmcu=atmega640 -Wall -O3 -Wl,-u,vfprintf -lprintf_flt -c main.c -o main.o
avr-gcc -mmcu=atmega640 -Wall -Wl,-u,vfprintf -lprintf_flt -lm -c command_handler.c -o command_handler.o
avr-gcc -mmcu=atmega640 -Wall -O3 -c pantilt.c -o pantilt.o
avr-gcc -mmcu=atmega640 -Wall -c analog.c -o analog.o
avr-gcc -mmcu=atmega640 -Wall -c usart.c -o usart.o
avr-gcc -mmcu=atmega640 -Wall -c array.c -o array.o
avr-gcc -mmcu=atmega640 -Wall -c modulus.c -o modulus.o

avr-gcc -mmcu=atmega640 -gstabs+ -combine main.c analog.c usart.c command_handler.c pantilt.c array.c modulus.c -o debug_format

avr-gcc -mmcu=atmega640 -Wall -Wl,-u,vfprintf -lprintf_flt -lm main.o analog.o usart.o command_handler.o pantilt.o array.o modulus.o -o gen.elf

avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex gen.elf EEPROM_DATA.eep

avr-objcopy -O ihex -j .text -j .data -j .bss gen.elf BOB4_AngleOverlay.hex


@echo off

echo.

avr-objdump -S debug_format gen.elf > output.lst

avr-size --mcu=atmega640 -C gen.elf

echo.
echo Size by section:
echo.

avr-size --mcu=atmega640 -A gen.elf

del gen.elf
del /q /f *.o
pause