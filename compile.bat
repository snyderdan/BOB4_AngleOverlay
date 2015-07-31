avr-gcc -mmcu=atmega640 -Wall -Wl,-u,vfprintf -lprintf_flt -O3 -c main.c -o main.o
avr-gcc -mmcu=atmega640 -Wall -Wl,-u,vfprintf -lprintf_flt -lm -c command_handler.c -o command_handler.o
avr-gcc -mmcu=atmega640 -Wall -c pantilt.c -o pantilt.o
avr-gcc -mmcu=atmega640 -Wall -c analog.c -o analog.o
avr-gcc -mmcu=atmega640 -Wall -c usart.c -o usart.o
avr-gcc -mmcu=atmega640 -Wall -c array.c -o array.o

avr-gcc -mmcu=atmega640 -Wall -Wa,-adhlns=output.lst -c pantilt.c main.o command_handler.o analog.o usart.o array.o -o pantilt.o

avr-gcc -mmcu=atmega640 -Wall -Wl,-u,vfprintf -lprintf_flt -lm main.o analog.o usart.o command_handler.o pantilt.o array.o -o gen.elf

avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex gen.elf EEPROM_DATA.eep

avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature gen.elf BOB4_AngleOverlay.hex

avr-size --mcu=atmega640 -C gen.elf

@echo off
del gen.elf
del /q /f *.o
pause