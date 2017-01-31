#ifndef SDR1KUSB_IMPORTS
#define SDR1KUSB_API __declspec(dllexport)
#else
#define SDR1KUSB_API __declspec(dllimport)
#endif

extern "C" SDR1KUSB_API int Sdr1kOpen(char* appName, unsigned int config);
extern "C" SDR1KUSB_API int Sdr1kClose(int deviceID);
extern "C" SDR1KUSB_API int Sdr1kSetNotify(int deviceID, void *hEvent);
extern "C" SDR1KUSB_API int Sdr1kGetStatusPort(int deviceID);
extern "C" SDR1KUSB_API int Sdr1kLatch(int deviceID, unsigned char latch, unsigned char data);
extern "C" SDR1KUSB_API int Sdr1kDDSReset(int deviceID);
extern "C" SDR1KUSB_API int Sdr1kDDSWrite(int deviceID, unsigned char addr, unsigned char data);
extern "C" SDR1KUSB_API int Sdr1kSRLoad(int deviceID, unsigned char reg, unsigned char data);
extern "C" SDR1KUSB_API int Sdr1kGetADC(int deviceID);
