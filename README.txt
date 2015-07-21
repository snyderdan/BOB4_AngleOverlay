//Prevision Systems LLC
//Project: Pan/Tilt video overlay
//Written by Dan Snyder
//Version 1 compiled 7/2/2015
//Compiled using GCC version 4.3.3 (WinAVR 20100110)

Hardware Info:

This code is written for the BUD board, coupled with the BOB-4 and is currently written
to overlay the current pan and tilt angles of the camera on the screen. The PAN analog angle should
be connected to ADC0 (pin 13 on the GPIO header) and the TILT analog angle should be connected
to ADC3 (pin 19 on the GPIO header). The BUD board uses the ATmega640 AVR microcontroller. 


Command Basics:

This code is compiled for the ATmega640 that is on the BUD board. Once the code has been
flashed to the controller, it will be able to recieve commands through a serial connection
for calibration purposes. The command formatting is as follows:

    <CSI>[cmd][operands]
    
Where <CSI> is the control sequence inducer as-per the BOB-4 (<CSI> is currently equal to 0x1B, 0x5B or "\e[")
and then cmd is one of the following characters:

    Z   - set zero-point of [operand 0] 
    C   - enter calibration of [operand 0]
    R   - set mode of [operand 0] to relative
    A   - set mode of [operand 0] to absolute
    T   - toggle mode of [operand 0]
    Q   - query raw analog values
    P   - view parameters (slopes, offsets, and ranges)
    
Current valid operands are:
    
    P   - Pan axis      - valid for all commands
    T   - Tilt axis     - valid for all commands
    
Any combination of operands can be sent. Options may be added to change the angle range, set time and date,
etc. which will have operands. 

Example Commands:

    Command         Description                         Terminal (Terminal by Br@y++)
    <CSI>CP         calibrate pan axis                  "$1B[CP"
    <CSI>APT        set pan & tilt to absolute mode     "$1B[APT" or "$1B[ATP"
    <CSI>P          view pan & tilt parameters          "$1B[P"


Calibration and Modes:

PAN and TILT should be calibrated first, then after that the zero points and angle mode can be set. 
Here is a break down of the calibration modes of PAN and TILT as well as the different angle modes:

PAN CALIBRATION:
    This code has default limits which will be "close enough" to have an idea of the camera angle. In order
    to calibrate, watch the angle until you find the area that the angle rolls over from 0 to 360 or vice-versa.
    Go slightly to one side of this area, then send the CALIBRATE PAN command. Slowly rotate the camera so that it
    passes over the roll-over point, and the ATmega640 will automatically detect the highest and lowest values. 
    The code is looking for a major change between the highest and lowest values, so if the CALIBRATE PAN command
    is sent and the camera is rotated away from the roll-over point, the code will most likely find an acceptable
    high and low value before it actually gets to the roll-over point. 
    
TILT CALIBRATION:
    This code has default limits which will be "close enough" to have an idea of the camera angle. In order
    to calibrate, start with the tilt at 0 degrees (straight forward). Send the CALIBRATE TILT command and then
    rotate all the way forward (which should be positive 120 degrees), then rotate all the way backwards 
    (-120 degrees). The code first records the highest value it encounters, and then records the lowest value
    once it begins to see values that are less than the current highest. 
    
ABSOLUTE ANGLE MODE:
    Absolute angle mode simply ranges from 0-360 degrees. So going forwards would start at 1 degree and count up,
    and going backwards would start at 359 degrees then count down.  
    
RELATIVE ANGLE MODE:
    Relative angle mode allows for negative angles to be displayed between -180 and 180. When angle modes are
    switched, the zero point remains the same. 0-180 degrees will display the same in both modes, with the
    only difference being 180 to 360 now being equal to -180 to 0. 
