#include "StdAfx.h"
#include "si570usb.h"
#include "usb.h"
#include <math.h>
#include <cstdio>

usb_dev_handle* hdev = NULL;

bool cap_3500ppmtune;
bool cap_setfreqallin;

int cfg_FWVersion;
int cfg_I2CAddr;
int cfg_FMult;
int cfg_FDiv;
int cfg_FOffset;

unsigned __int64 si570_regs;
double f_si570;
double f_si570_center;

int si570usb_open()
{
	// open usb device
    struct usb_bus *bus = NULL;
    struct usb_device *dev = NULL;    
	int myvid = DG8SAQ_VENDOR_ID;
	int mypid = DG8SAQ_PRODUCT_ID;

	if(hdev) {
		return 0;
	}

    usb_init();
    usb_find_busses();
    usb_find_devices();
    
    for (bus = usb_get_busses(); bus && !hdev; bus = bus->next)
    {
        for (dev = bus->devices; dev && !hdev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == myvid 
                    && dev->descriptor.idProduct == mypid)
            {
                hdev = usb_open(dev);
            }
        }
    }
    
    if(!hdev) {
//		::MessageBox(NULL, "Unable to find QRP2008", "USB Error", MB_OK);
		return -1;
    }
//	else {
//        if (usb_set_configuration(hdev, 1) != 0){
//            usb_close(hdev);
//            return -1;
//        }
//        if (usb_claim_interface(hdev, 0) != 0) {
//            usb_close(hdev);
//            return -1; 
//        }
//    }

	// reading firmware configuration data
	int val;
	val = si570usb_get_version();
	cfg_FWVersion = (val!=0xff) ? val : 0x14;

	val = si570usb_get_eeprom(EEPROM_SI570_I2CADDR);
	cfg_I2CAddr = (val!=0xff) ? val : 0x55;

	val = si570usb_get_eeprom(EEPROM_F_MULT);
	cfg_FMult = (val!=0xff) ? val : 0x04;

	val = si570usb_get_eeprom(EEPROM_F_DIV);
	cfg_FDiv = (val!=0xff) ? val : 0x01;

	val = si570usb_get_eeprom32(EEPROM_F_OFFSET);
	cfg_FOffset = (val!=0xffffffff) ? val : 0x00;

	// discover firmware capabilities
	cap_3500ppmtune = (si570usb_getregs() != 0);	// capability for si570 register access (required for 3500ppm tuning)

	// Work out if the AVR is capable of performing all tuning calculations
	// and actions on its own. This is difficult because it does not return a
	// status, so we have to set the Si570 registers to an improbable value,
	// try executing the command, and then check that the Si570 values have changed.
	// In practice, we set the N1 regster to an invalid, but harmless, even value.

	si570_regs = si570usb_getregs();
	if (si570_regs == 0)
	{
		cap_setfreqallin = FALSE;	// If we can't read the Si570 regs then no way to tell
	} else {						//  if the extended tuning function is supported ... so assume not.
		si570_regs = si570_regs & 0xFFBFFFFFFFFF; // Force N1 reg to be even
		si570usb_setregs();						// ... and write back to Si570

		si570usb_setfreqallin((DWORD)7000000);	// Try to set a sensible frequency...
		si570_regs = si570usb_getregs();		// and read the registers again.

		cap_setfreqallin = ((si570_regs & 0x004000000000) != 0); // Check if N1 is odd again now
	}

	// initialize firmware
	si570usb_set_rxtx(false);	// required for old firmware

	if(si570usb_get_eeprom(EEPROM_INIT_SI570) != 0xFF){
		// disable storage of latest frequency in EEPROM
		si570usb_set_eeprom(EEPROM_INIT_SI570, 0xFF);
	}

	// initialize driver
	f_si570 = 0.0;
	f_si570_center = 0.0;
	si570_regs = si570usb_getregs();

	return 0;
}

int si570usb_close()
{
	//si570usb_reset();
	//usb_release_interface(hdev, 0);
	int ret = usb_close(hdev);
	hdev = NULL;
	return ret;
}

void si570usb_reset()
{
	char ret;
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_RESET, 0, 0, &ret, 1, 3000);
}

int si570usb_set_eeprom(int addr, int val)
{
	char r;
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_SET_EEPROM_BYTE, addr, val, &r, 1, USB_TIMEOUT);
	return r;
}

BYTE si570usb_get_eeprom(int addr)
{
	char ret;
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_EEPROM_BYTE, addr, 0, &ret, 1, USB_TIMEOUT);
	return (BYTE)ret;
}

int si570usb_get_eeprom32(int addr)
{
	int ret;
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_EEPROM_BYTE, addr+0, 0, &((char*)&ret)[3], 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_EEPROM_BYTE, addr+1, 0, &((char*)&ret)[2], 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_EEPROM_BYTE, addr+2, 0, &((char*)&ret)[1], 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_EEPROM_BYTE, addr+3, 0, &((char*)&ret)[0], 1, USB_TIMEOUT);
	return ret;
}

int si570usb_get_version()
{
	char ret;
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_FWVERSION, 0, 0, &ret, 1, USB_TIMEOUT);
	return (int)ret;
}

unsigned __int64 swap6(unsigned __int64 i)
{
		unsigned __int64 swap;
		((char*)&swap)[7] = 0;
		((char*)&swap)[6] = 0;
		((char*)&swap)[5] = ((char*)&si570_regs)[0];
		((char*)&swap)[4] = ((char*)&si570_regs)[1];
		((char*)&swap)[3] = ((char*)&si570_regs)[2];
		((char*)&swap)[2] = ((char*)&si570_regs)[3];
		((char*)&swap)[1] = ((char*)&si570_regs)[4];
		((char*)&swap)[0] = ((char*)&si570_regs)[5];
		return swap;
}

unsigned __int64 si570usb_getregs()
{
	if(usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_SI570_REGS, 0x700 + cfg_I2CAddr, 0, (char*)&si570_regs, 6, USB_TIMEOUT) == 6){
		si570_regs = swap6(si570_regs);
	} else {
		si570_regs = 0;
	}
	return si570_regs;
}

int si570usb_setregs()
{
	char r;
	unsigned __int64 regs = swap6(si570_regs);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_I2C_START, 0, 0, &r, 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_I2C_WR32, 0x07<<8 | (cfg_I2CAddr<<1)&0xfe, (int) regs, &r, 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_I2C_WR32, (int) regs>>16, (int) regs>>32, &r, 1, USB_TIMEOUT);
	usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_I2C_STOP, 0, 0, &r, 1, USB_TIMEOUT);
	return 0;
}

int si570usb_setfreq(double freq)
{
	int ret = 0;

	if(cap_setfreqallin){
		ret = si570usb_setfreqallin((DWORD)freq);
	} else {
		// apply multiplier, divider, offset
		double f = ((double)cfg_FMult) / ((double)cfg_FDiv) * (freq + cfg_FOffset);
		// test if clickless tuning is possible
		if(cap_3500ppmtune && fabs(f - f_si570_center) < (f_si570_center * 3500e-6)){
			// small change
			si570_regs = si570_regs&0xFFC000000000 | (unsigned __int64)((double)(si570_regs&0x003FFFFFFFFF) * (f / f_si570));
			si570usb_setregs();
		} else {
			// large change
			DWORD dwF = (DWORD) (f * (1e-6 * (1 << 21)) );
			ret = usb_control_msg(hdev, REQUEST_TYPE_OUT, CMD_SET_SI570_FREQ, 0x700 + cfg_I2CAddr, 0, (char *)&dwF, 4, USB_TIMEOUT) == sizeof(dwF) ? 0 : -1;
			f_si570_center = f;
			if(cap_3500ppmtune)
				si570usb_getregs();
		}
		f_si570 = f;
	}
	return ret;
}

int si570usb_set_rxtx(bool tx)
{
	char ret;
	return usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_SET_RXTX, tx, 0, &ret, 1, USB_TIMEOUT) == 1 ? 0 : -1;
}

int si570usb_get_cwkeys()
{
	char ret;
	static int poll_count=0, last_ret=0; // Experimental timing change
	#define POLL_SLOWDOWN 5		// Only poll AVR one time in every 5 polls from PowerSDR
	
	poll_count++;
	if (poll_count >= POLL_SLOWDOWN)
	{
		poll_count = 0;
		usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_CWKEYS, 0, 0, &ret, 1, USB_TIMEOUT);
		last_ret = ret;
	} else {
		ret = last_ret;				// If not ready to check AVR again, just report same as last time;
	}
	return ((ret & FLAG_CW_DOT) ? 0x00 : 0x02) | ((ret & FLAG_CW_DASH) ? 0x00 : 0x01);  // M0PUB: Inverted state of CW inputs
}

int si570usb_setfreqallin(DWORD freq)	//>v1.4
{
	return usb_control_msg(hdev, REQUEST_TYPE_OUT, CMD_SET_FREQ_ALLIN, 0x700 + cfg_I2CAddr, 0, (char *)&freq, 4, USB_TIMEOUT) == sizeof(freq) ? 0 : -1;
}

int si570usb_set_gain(int gain)	//>v1.4
{
	char ret;
	return usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_SET_GAIN, gain, 0, &ret, 1, USB_TIMEOUT) == 1 ? 0 : -1;
}

int si570usb_getADC()
{
	char ADCres[2];

	if(usb_control_msg(hdev, REQUEST_TYPE_IN, CMD_GET_ADC_RESULTS, 0, 0, ADCres, 2, USB_TIMEOUT) == 2)
	{
		ADCres[0] = ADCres[0] >> 1;				// PowerSDR expects 7-bit value
		ADCres[1] = ADCres[1] >> 1;				// PowerSDR expects 7-bit value
		return (ADCres[0] | (ADCres[1] << 8));
	}

	// If anything other than two bytes returned, assume AVRdoes does not have this capability
	return 0;
}
