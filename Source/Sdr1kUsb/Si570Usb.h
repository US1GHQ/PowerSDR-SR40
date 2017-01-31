#pragma once

#define USB_TIMEOUT			500

#define REQUEST_TYPE_IN		(USB_TYPE_VENDOR|USB_ENDPOINT_IN)
#define REQUEST_TYPE_OUT	(USB_TYPE_VENDOR|USB_ENDPOINT_OUT)

// G8SAQ-I2C device identifier
#define DG8SAQ_VENDOR_ID	0x16C0;
#define DG8SAQ_PRODUCT_ID	0x05DC;

// DG8SAQ-I2C USB commands
#define CMD_ECHO				0x00	// diagnosis
#define CMD_SET_DDRB			0x01	// set DDRB
#define CMD_GET_PINB			0x02	// read PINB
#define CMD_GET_PORTB			0x03	// read PORTB
#define CMD_SET_PORTB			0x04	// set PORTB
#define CMD_I2C_START			0x05	// send I2C start sequence
#define CMD_I2C_STOP			0x06	// send I2C stop sequence
#define CMD_I2C_WR8				0x07	// send byte to I2C
#define CMD_I2C_WR16			0x08	// send word to I2C
#define CMD_I2C_WR32			0x09	// send dword to I2C
#define CMD_I2C_WR16SS			0x0a	// send word to I2C with start and stop sequence
#define CMD_I2C_RD16SS			0x0b	// receive word to I2C with start and stop sequence
#define CMD_SET_I2C_TIMECONST	0x0c	// set I2C clock frequency
#define CMD_RESET				0x0f	// reset
#define CMD_SET_EEPROM_BYTE		0x10	// write byte to EEPROM
#define CMD_GET_EEPROM_BYTE		0x11	// read byte from EEPROM
#define CMD_SET_SI570_REG		0x20	// SI570 write byte to register
#define CMD_GET_SI570_REG		0x21	// SI570 read byte from register
#define CMD_SI570_FREEZE		0x22	// SI570 freeze DCO
#define CMD_SI570_UNFREEZE		0x23	// SI570 unfreeze DCO
#define CMD_SET_SI570_REGS		0x30	// set SI570 by registers with freeze/unfreeze DCO
#define CMD_SET_SI570_FREQ		0x32	// set SI570 by frequency in 2^21 with freeze/unfreeze DCO
#define CMD_SET_SI570_FXTAL		0x33	// set crystal frequency
#define CMD_GET_SI570_REGS		0x3F	// read all SI570 registers
#define CMD_GET_I2C_ERRORS		0x40	// return number of I2C errors
#define CMD_SET_SI570_PRESET	0x41	// set/clear frequency memory on startup
#define CMD_SET_RXTX			0x50	// set RX/TX line and read CW keys
#define CMD_GET_CWKEYS			0x51	// read CW keys
// Enhanced DG8SAQ-I2C USB commands (firmware >v1.4)
#define CMD_SET_FREQ_ALLIN		0x38	// set SI570 by frequency in Hz while taking multiplier, divider, offset values into account and set the filter select lines appropriately
#define CMD_SET_GAIN			0x55	// set gain (bit1=attenuator; bit0=gain) (0=off, 1=low, 2=medium, 3=high)
#define CMD_GET_FWVERSION		0xFF	// get firmware version

// Flags
#define FLAG_CW_DOT				0x10	// CW dot key input /PB5 (for straight key/iambic keyer)
#define FLAG_CW_DASH			0x02	// CW dash key input SDA (for iambic keyer)

// DG8SAQ-I2C EEPROM content
#define EEPROM_RC_CAL_VALUE		0x00	// RC calibration value of AVR internal oscillator
#define EEPROM_F_CAL_STATUS		0x01	// Enable use of calibrated value of Si570 Fxtal (default=disabled)
#define	EEPROM_F_CRYST			0x02	// Calibrated value of Si570 Fxtal (int32)
#define EEPROM_INIT_SI570		0x06	// I2C address of Si570 which enables store latest frequency and restore on init (default=disabled)
#define EEPROM_INIT_FREQ		0x07	// Frequency to restore on init (6 bytes)
// Enhanced use of EEPROM content as accessed by driver in case of old firmware (SHOULD be compatible with firmware >v1.4)
#define EEPROM_SI570_I2CADDR	0x10	// I2C address of SI570 to be used by host (default=0x55)
#define EEPROM_F_MULT			0x11	// Frequency multiplier value (default=0x04)
#define EEPROM_F_DIV			0x12	// Frequency divider value (default=0x01)
#define EEPROM_F_OFFSET			0x13	// Frequency offset in Hz (int32) (default=0x00000000)
#define EEPROM_FWVERSION		0xFF	// Firmware version (high octet is major, low octet is minor) (default=0x14)

// Prototypes
int si570usb_open();
int si570usb_close();
void si570usb_reset();
int si570usb_set_eeprom(int addr, int val);
BYTE si570usb_get_eeprom(int addr);
int si570usb_get_eeprom32(int addr);
int si570usb_get_version();
unsigned __int64 si570usb_getregs();
int si570usb_setregs();
int si570usb_setfreq(double f);
int si570usb_set_rxtx(bool tx);
int si570usb_get_cwkeys();
int si570usb_setfreqallin(DWORD f);	//>v1.4
int si570usb_set_gain(int gain);	//>v1.4