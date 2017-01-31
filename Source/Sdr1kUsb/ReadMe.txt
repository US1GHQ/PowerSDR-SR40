Sdr1kUsb driver for DG8SAQ SI570 USB interface

Sdr1kUsb is a SDR-1000 compatible driver for PowerSDR, and 
controls a SI570-XO via an AVR USB interface (DG8SAQ-I2C). 
The AVR USB interface [1] is used in various SDRs such as 
Softrock [2].

Installation instructions:
1. Copy Sdr1kUsb.dll to PowerSDR directory;
2. Select SDR-1000 model (Setup > Hardware Config > Radio Model);
3. Enable USB Adapter (Setup > Hardware Config > Hardware Setup).

Enhanced configuration instructions:
1. Run SI570_USB_Test.exe application, as bundled in the firmware package [1];
2. set in "request" field a EEPROM write command 10 (HEX), or EEPROM read command 11 (HEX);
3. set in "value" field the EEPROM address (HEX)  (value 00-FF without 0x);
4. set in "index" field the EEPROM value (HEX) to be written in case of a write command;
5. press "USB_control_msg   data to host" button to submit command;
6. read EEPROM value of "USB transfer return value" field in case of a read command, or read 00 in case of a write command;
7. EEPROM setting may be refreshed in PowerSDR by restarting or by re-enabling USB Adapter (Setup > Hardware Config > Hardware Setup).

The driver may be configured by writing a value at a specific 
EEPROM address. If uninitialized (value=FF) the driver use a default.

Address   Description
0x00      RC calibration value of AVR internal oscillator
0x01      Enable use of calibrated value of Si570 Fxtal (default=disabled)
0x02-0x05 Calibrated value of Si570 Fxtal
0x06      I2C address of Si570 which enables store latest frequency and restore on init (default=disabled)
0x07-0x0c Frequency to restore on init
0x10      I2C address of SI570 to be used by host (default=0x55)
0x11      Frequency multiplier value (default=0x04)
0x12      Frequency divider value (default=0x01)
0x13-0x16 Frequency offset in Hz (default=0x00000000)

73, Guido
PE1NNZ


[1] AVR USB interface (DG8SAQ-I2C), http://www.mydarc.de/dg8saq/SI570/index.shtml

[2]	Softrock Lite+USB Xtall v9.0, http://groups.yahoo.com/group/softrock40/files/Lite%2BUSB%20Xtall%20v9.0%20docs/