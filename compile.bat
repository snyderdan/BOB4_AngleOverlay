avr-gcc -mmcu=atmega640 -Wl,-u,vfprintf -lprintf_flt -O3 -c main.c -o main.o
avr-gcc -mmcu=atmega640 -Wl,-u,vfprintf -lprintf_flt -c command_handler.c -o command_handler.o
avr-gcc -mmcu=atmega640 -c analog.c -o analog.o
avr-gcc -mmcu=atmega640 -c usart.c -o usart.o
avr-gcc -mmcu=atmega640 -c pantilt.c -o pantilt.o
avr-gcc -mmcu=atmega640 main.o analog.o usart.o command_handler.o pantilt.o -o gen.elf
avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature gen.elf BOB4_AngleOverlay.hex
del gen.elf
del /q /f *.o
pause