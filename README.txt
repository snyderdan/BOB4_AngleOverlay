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

	A controlling board/computer can also communicate to the BOB-4 through the PC serial port (J4) 
	using RS232. The controller for this specific application will be updating the raw pan and
	tilt values through commands instead of reading them through the analog lines. This will be
	the default configuration. Once active, a command can be sent to set it to analog source again.
	
	
Default Configuration:
	
	The default configuration is relative angle mode for both pan and tilt, with the input source
	set to digital.


Command Summary:

	This code is compiled for the ATmega640 that is on the BUD board. Once the code has been
	flashed to the controller, it will be able to recieve commands through a serial connection
	for calibration purposes. The command formatting is as follows:

		<CSI>[cmd][operands]<CST>
		
	Where <CSI> is the control sequence introducer which is currently equal to 0xC3. <CST> is
	command sequence terminator which is equal to 0x5D. Valid commands are:

		Z	- set zero-point of pan/tilt (2 operands max)
		C	- enter calibration of pan/tilt (1 operand max)
		R	- set mode of pan/tilt to relative (2 operands max)
		A	- set mode of pan/tilt to absolute (2 operands max)
		T	- toggle mode of pan/tilt (2 operands max)
		G	- get raw pan and tilt values (0 operands)
		Q	- query sampled pan and tilt values (0 operands)
		P	- view parameters (slopes, offsets, and ranges) (0 operands)
		V	- set input source to analog (0 operands)
		D	- set input source to digital (0 operands)
		S	- set pan and tilt values (strictly 8 operands)
	
Operand Information:

	If any command recieves too many operands, or too few if operand count is strict, then the
	command is disregarded and the program begins scanning for a valid <CSI> starting at the byte
	that follows the end of the invalidated command. If a <CSI> is sent with an invalid command
	following it, the program begins scanning for a valid <CSI> starting at the byte following the
	invalidated command byte. 
	
	Z, R, A, and T commands accept 'P' for pan, and 'T' for tilt as operands. You can use
		either or both operands for these commands.
		
	C is special. It accepts 'P' for pan, 'U' for tilt Upper limit, and 'L' for tilt Lower limit.
		Only one calibration method can be entered at a time. See the Calibration section for more 
		information.
		
	S command takes strictly 8 bytes. These bytes must be ASCII hex characters ('0'-'9' and 'A'-'F')
		The command structure is as follows:
			Byte	Part						Value
			1		Header						0xC3
			2		Command						'S'
			3		Pan high-high nibble		'0'-'9' and 'A'-'F'
			4		Pan high-low nibble			^
			5		Pan low-high nibble			^
			6		Pan low-low nibble			^
			7		Tilt high-high nibble		^
			8		Tilt high-low nibble		^
			9		Tilt low-high nibble		^
			10		Tilt low-low nibble			^
			11		Terminator					0x5D
			
	The remaining commands do not take any operands. Providing any operands will invalidate the command.
	A termination byte is still required for these commands.

Example Commands:

	Command			Description							C string command
	<CSI>CP<CST>	calibrate pan axis					"\xC3CP\x5D"
	<CSI>APT<CST>	set pan & tilt to absolute mode		"\xC3APT\x5D" or "\xC3ATP\x5D"
	<CSI>P<CST>		view pan & tilt parameters			"\xC3P\x5D"
	
Examples of Invalidating Commands:

	Invalidated Command			C string command	Problem
	<CSI>CPT					"\xC3CPT..."		Too many arguments
	<CSI>APTP					"\xC3APTP..."		Too many arguments
	<CSI>S1F<CST>				"\xC3S1F\x5D"		Too few arguments
	<CSI>F						"\xC3F..."			Invalid command byte


Calibration:

	***TILT CALIBRATION MODE DOCUMENTED BELOW IS OUTDATED***

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
	
Angle Modes:
	
	ABSOLUTE ANGLE MODE:
		Absolute angle mode simply ranges from 0-360 degrees. So going forwards would start at 1 degree and count up,
		and going backwards would start at 359 degrees then count down.  
		
	RELATIVE ANGLE MODE:
		Relative angle mode allows for negative angles to be displayed between -180 and 180. When angle modes are
		switched, the zero point remains the same. 0-180 degrees will display the same in both modes, with the
		only difference being 180 to 360 now being equal to -180 to 0. 
