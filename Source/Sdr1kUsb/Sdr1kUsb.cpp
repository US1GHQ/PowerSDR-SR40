// Sdr1kUsb.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Sdr1kUsb.h"
#include "si570usb.h"
#include <math.h>
#include <cstdio>
#include <assert.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

extern "C" SDR1KUSB_API int Sdr1kOpen(char* appName, unsigned int config){
//	::MessageBox(NULL, appName, appName, 0);
	return si570usb_open();
};

extern "C" SDR1KUSB_API int Sdr1kClose(int deviceID){
	return si570usb_close();
};

extern "C" SDR1KUSB_API int Sdr1kSetNotify(int deviceID, void *hEvent){
	return 0;
};

extern "C" SDR1KUSB_API int Sdr1kGetStatusPort(int deviceID){
#define SDR1K_STATUS_INV_X2_11	0x80
#define SDR1K_STATUS_ATUFB	0x40
#define SDR1K_STATUS_DOTPTT	0x20
#define SDR1K_STATUS_DASH	0x10
#define SDR1K_STATUS_X2_12	0x08

	return SDR1K_STATUS_INV_X2_11 | (si570usb_get_cwkeys()<<4);
};

extern "C" SDR1KUSB_API int Sdr1kLatch(int deviceID, unsigned char latch, unsigned char data){
#define SDR1K_LATCH_EXT	0x01
#define SDR1K_EXT_X2_1	0x01
#define SDR1K_EXT_X2_2	0x02
#define SDR1K_EXT_X2_3	0x04
#define SDR1K_EXT_X2_4	0x08
#define SDR1K_EXT_X2_5	0x10
#define SDR1K_EXT_X2_6	0x20
#define SDR1K_EXT_X2_7	0x40
#define SDR1K_EXT_GAIN	0x80

#define SDR1K_LATCH_BPF	0x02
#define SDR1K_BPF_BPF0	0x01
#define SDR1K_BPF_BPF1	0x02
#define SDR1K_BPF_BPF3	0x04
#define SDR1K_BPF_BPF2	0x08
#define SDR1K_BPF_BPF4	0x10
#define SDR1K_BPF_BPF5	0x20
#define SDR1K_BPF_TR	0x40
#define SDR1K_BPF_MUTE	0x80

#define SDR1K_LATCH_IC11	0x100
#define SDR1K_LATCH_IC7		0x101
#define SDR1K_IC7_AMP2		0x01
#define SDR1K_IC7_AMP1		0x02
#define SDR1K_IC7_XVRX		0x04
#define SDR1K_IC7_XVEN		0x08
#define SDR1K_IC7_ATTEN		0x10
#define SDR1K_IC7_IMPR		0x20
#define SDR1K_IC7_SPARE		0x40
#define SDR1K_IC7_IMP		0x80
#define SDR1K_LATCH_IC10	0x102
#define SDR1K_LATCH_IC9		0x103

	static unsigned char latchreg[0x200];
	latchreg[latch] = data;

#ifdef notdef
	// rfe
	static unsigned char sreg;
	if(!(latchreg[SDR1K_LATCH_BPF] & 0x04)) sreg = 0;
	if((latch == SDR1K_LATCH_BPF) && (data & 0x02)) sreg = (sreg << 1) | (latchreg[SDR1K_LATCH_BPF] & 0x01);
	if((latch == SDR1K_LATCH_BPF) && !(data & 0x20)){
		int y = ((~data) & 0x0c)>>3;
		latchreg[0x100+y] = sreg;
	}
#endif

	// decoding
	bool ptt = ((latchreg[SDR1K_LATCH_BPF] & SDR1K_BPF_TR) != 0);
	si570usb_set_rxtx(ptt);

	bool gain = ((latchreg[SDR1K_LATCH_EXT] & SDR1K_EXT_GAIN) != 0);
	bool atten = false;	//((latchreg[SDR1K_LATCH_IC7] & SDR1K_IC7_ATTEN) != 0);
	si570usb_set_gain((gain ? 1 : 0) | (atten ? 2 : 0));

	return 0;
};

extern "C" SDR1KUSB_API int Sdr1kDDSReset(int deviceID){
	return 0;
};

extern "C" SDR1KUSB_API int Sdr1kDDSWrite(int deviceID, unsigned char addr, unsigned char data){
	static unsigned char ad9854reg[256];
	ad9854reg[addr] = data;
	if(addr == 0x09){
		unsigned __int64 ftw = (unsigned __int64)ad9854reg[4]<<(5*8)|(unsigned __int64)ad9854reg[5]<<(4*8)|(unsigned __int64)ad9854reg[6]<<(3*8)|(unsigned __int64)ad9854reg[7]<<(2*8)|(unsigned __int64)ad9854reg[8]<<(1*8)|(unsigned __int64)ad9854reg[9]<<(0*8);
		double f = ((double)ftw) * (double)200.0e6 /  (double)pow((double)2,(double)48);
		if(f != 0)
			return si570usb_setfreq(f);
	}
	return 0;
};

extern "C" SDR1KUSB_API int Sdr1kSRLoad(int deviceID, unsigned char reg, unsigned char data){
	return 0;
};

extern "C" SDR1KUSB_API int Sdr1kGetADC(int deviceID){
	return si570usb_getADC();
};
