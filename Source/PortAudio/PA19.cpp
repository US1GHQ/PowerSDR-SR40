/*   Port Audio Library Dll Creation Program  -- Copyright 2004 FlexRadio Systems  /*
 *   Written by Eric Wachsmann
 */

#include <windows.h>
#include "portaudio.h"
//#include "PA19.h"

#ifdef PA19_EXPORTS
#define PA19_API __declspec(dllexport)
#else
#define PA19_API __declspec(dllimport)
#endif

#define CCONV __stdcall

int WINAPI DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
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


#ifdef __cplusplus
extern "C" {
#endif

void *streamCB;
void *streamFinishedCB;
void *streamCB2;
void *streamFinishedCB2;

int __cdecl myStreamCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	typedef int (__stdcall *FNPTR)(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
	FNPTR fnptr = (FNPTR)streamCB;
	return fnptr(input, output, frameCount, timeInfo, statusFlags, userData);
}

int __cdecl myStreamCallback2(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	typedef int (__stdcall *FNPTR)(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
	FNPTR fnptr = (FNPTR)streamCB2;
	return fnptr(input, output, frameCount, timeInfo, statusFlags, userData);
}

void __cdecl myStreamFinishedCallback(void *userData)
{
	typedef int (__stdcall *FNPTR)(void *userData);
	FNPTR fnptr = (FNPTR)streamFinishedCB;
	fnptr(userData);
}

void __cdecl myStreamFinishedCallback2(void *userData)
{
	typedef int (__stdcall *FNPTR)(void *userData);
	FNPTR fnptr = (FNPTR)streamFinishedCB2;
	fnptr(userData);
}

PA19_API int CCONV PA_GetVersion()
{
	return Pa_GetVersion();
}

PA19_API const char* CCONV PA_GetVersionText()
{
	return Pa_GetVersionText();
}

PA19_API const char* CCONV PA_GetErrorText(PaError errorCode) 
{  
	return Pa_GetErrorText(errorCode);
}

PA19_API PaError CCONV PA_Initialize()
{
	return Pa_Initialize();
}

PA19_API PaError CCONV PA_Terminate()
{
	return Pa_Terminate();
}

PA19_API PaHostApiIndex CCONV PA_GetHostApiCount()
{
	return Pa_GetHostApiCount();
}

PA19_API PaHostApiIndex CCONV PA_GetDefaultHostApi()
{
	return Pa_GetDefaultHostApi();
}

PA19_API const PaHostApiInfo* CCONV PA_GetHostApiInfo(PaHostApiIndex hostApi)
{
	return Pa_GetHostApiInfo(hostApi);
}

PA19_API PaHostApiIndex CCONV PA_HostApiTypeIdToHostApiIndex(PaHostApiTypeId type)
{
	return Pa_HostApiTypeIdToHostApiIndex(type);
}

PA19_API PaDeviceIndex CCONV PA_HostApiDeviceIndexToDeviceIndex(PaHostApiIndex hostApi, int hostApiDeviceIndex)
{
	return Pa_HostApiDeviceIndexToDeviceIndex(hostApi, hostApiDeviceIndex);
}

PA19_API const PaHostErrorInfo* CCONV PA_GetLastHostErrorInfo()
{
	return Pa_GetLastHostErrorInfo();
}

PA19_API PaDeviceIndex CCONV PA_GetDeviceCount()
{
	return Pa_GetDeviceCount();
}

PA19_API PaDeviceIndex CCONV PA_GetDefaultInputDevice()
{
	return Pa_GetDefaultInputDevice();
}

PA19_API PaDeviceIndex CCONV PA_GetDefaultOutputDevice()
{
	return Pa_GetDefaultOutputDevice();
}

PA19_API const PaDeviceInfo* CCONV PA_GetDeviceInfo(PaDeviceIndex device)
{
	return Pa_GetDeviceInfo(device);
}

PA19_API PaError CCONV PA_IsFormatSupported(const PaStreamParameters *inputParameters,
									  const PaStreamParameters *outputParameters, double sampleRate) 
{
	return Pa_IsFormatSupported(inputParameters, outputParameters, sampleRate);
}

//[patch_11
//#define PATCH_11	1
#if (PATCH_11)
static HANDLE hEvent[2];
static int input_four_channel[4];
static int output_four_channel[4];

int __cdecl myStreamCallbackAB(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	if(userData == 0){
		input_four_channel[0] = ((int *)input)[0];
		input_four_channel[1] = ((int *)input)[1];
		output_four_channel[2] = ((int *)output)[0];
		output_four_channel[3] = ((int *)output)[1];
		::SetEvent(hEvent[0]);
	} else {
		input_four_channel[2] = ((int *)input)[0];
		input_four_channel[3] = ((int *)input)[1];
		output_four_channel[0] = ((int *)output)[0];
		output_four_channel[1] = ((int *)output)[1];
		::SetEvent(hEvent[1]);
	}

	DWORD timeout = 500;

	DWORD res = ::WaitForMultipleObjects(2, hEvent, TRUE, timeout);
	if(res != WAIT_OBJECT_0 && res != WAIT_OBJECT_0 + 1) return paNoError;

	PaError ret = paNoError;
	if(userData == 0){
		typedef int (__stdcall *FNPTR)(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
		FNPTR fnptr = (FNPTR)streamCB;
		ret = fnptr(input_four_channel, output_four_channel, frameCount, timeInfo, statusFlags, userData);
		::SetEvent(hEvent[0]);
	} else {
		::SetEvent(hEvent[1]);
	}

	res = ::WaitForMultipleObjects(2, hEvent, TRUE, timeout);
	if(res != WAIT_OBJECT_0 && res != WAIT_OBJECT_0 + 1) return paNoError;

	return ret;
}

static PaStream* streamA = NULL;
static PaStream* streamB = NULL;

PA19_API PaError CCONV PA_OpenStream(PaStream **stream, const PaStreamParameters *inputParameters,
							   const PaStreamParameters *outputParameters, double sampleRate,
							   unsigned long framesPerBuffer, PaStreamFlags streamFlags,
							   PaStreamCallback *streamCallback, void *userData)
{
	if(userData == 0 && inputParameters->channelCount == 4 && (*Pa_GetDeviceInfo(inputParameters->device)).maxInputChannels == 2)
	{
		PaError err;
		PaStreamParameters inputParameters2 = *inputParameters;
		PaStreamParameters outputParameters2 = *outputParameters;
		inputParameters2.channelCount = 2;
		outputParameters2.channelCount = 2;
		streamCB = streamCallback;
		err = Pa_OpenStream(&streamA, &inputParameters2, &outputParameters2, sampleRate, framesPerBuffer, streamFlags, myStreamCallbackAB, (void *)0);
		if(err != paNoError) return err;
		PaStreamParameters inputParameters3 = *inputParameters;
		PaStreamParameters outputParameters3 = *outputParameters;
		inputParameters3.channelCount = 2;
		outputParameters3.channelCount = 2;
		inputParameters3.device = Pa_GetDefaultInputDevice();
		outputParameters3.device = Pa_GetDefaultOutputDevice();
		err = Pa_OpenStream(&streamB, &inputParameters3, &outputParameters3, sampleRate, framesPerBuffer, streamFlags, myStreamCallbackAB, (void *)2);
		if(err != paNoError) return err;
		*stream = streamA;
		return paNoError;
	}

	if(userData)
	{
		streamCB2 = streamCallback;
		return Pa_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer,
			 streamFlags, myStreamCallback2, userData);
	}
	else
	{
		streamCB = streamCallback;
		return Pa_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer,
			 streamFlags, myStreamCallback, userData);
	}
}

PA19_API PaError CCONV PA_OpenDefaultStream(PaStream **stream, int numInputChannels, int numOutputChannels,
									  PaSampleFormat sampleFormat, double sampleRate,
									  unsigned long framesPerBuffer, PaStreamCallback *streamCallback,
									  void *userData)
{
	if(userData)
	{
		streamCB2 = streamCallback;
		return Pa_OpenDefaultStream(stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate,
				framesPerBuffer, myStreamCallback2, userData);
	}
	else
	{
		streamCB = streamCallback;
		return Pa_OpenDefaultStream(stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate,
				framesPerBuffer, myStreamCallback, userData);
	}
}

PA19_API PaError CCONV PA_CloseStream(PaStream *stream) 
{
	if(stream == streamA)
	{
		PaError err;
		err = Pa_CloseStream(streamA);
		if(err != paNoError) return err;
		err = Pa_CloseStream(streamB);
		if(err != paNoError) return err;
		return err;
	}
	return Pa_CloseStream(stream);
}

PA19_API PaError CCONV PA_SetStreamFinishedCallback(PaStream *stream, PaStreamFinishedCallback *streamFinishedCallback)
{
	streamFinishedCB = streamFinishedCallback;
	return Pa_SetStreamFinishedCallback(stream, myStreamFinishedCallback);
}

PA19_API PaError CCONV PA_StartStream(PaStream *stream)
{
	if(stream == streamA)
	{
		hEvent[0] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		hEvent[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		PaError err;
		err = Pa_StartStream(streamA);
		if(err != paNoError) return err;
		err = Pa_StartStream(streamB);
		if(err != paNoError) return err;
		return err;
	}
	return Pa_StartStream(stream);
}

PA19_API PaError CCONV PA_StopStream(PaStream *stream)
{
	if(stream == streamA)
	{
		PaError err;
		err = Pa_StopStream(streamA);
		if(err != paNoError) return err;
		err = Pa_StopStream(streamB);
		if(err != paNoError) return err;
		return err;
	}
	return Pa_StopStream(stream);
}

PA19_API PaError CCONV PA_AbortStream(PaStream *stream)
{
	if(stream == streamA)
	{
		::CloseHandle(hEvent[0]);
		::CloseHandle(hEvent[1]);

		PaError err;
		err = Pa_AbortStream(streamA);
		if(err != paNoError) return err;
		err = Pa_AbortStream(streamB);
		if(err != paNoError) return err;

		return err;
	}
	return Pa_AbortStream(stream);
}
#else
//patch_11]

PA19_API PaError CCONV PA_OpenStream(PaStream **stream, const PaStreamParameters *inputParameters,
							   const PaStreamParameters *outputParameters, double sampleRate,
							   unsigned long framesPerBuffer, PaStreamFlags streamFlags,
							   PaStreamCallback *streamCallback, void *userData)
{
	if(userData)
	{
		streamCB2 = streamCallback;
		return Pa_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer,
			 streamFlags, myStreamCallback2, userData);
	}
	else
	{
		streamCB = streamCallback;
		return Pa_OpenStream(stream, inputParameters, outputParameters, sampleRate, framesPerBuffer,
			 streamFlags, myStreamCallback, userData);
	}
}

PA19_API PaError CCONV PA_OpenDefaultStream(PaStream **stream, int numInputChannels, int numOutputChannels,
									  PaSampleFormat sampleFormat, double sampleRate,
									  unsigned long framesPerBuffer, PaStreamCallback *streamCallback,
									  void *userData)
{
	if(userData)
	{
		streamCB2 = streamCallback;
		return Pa_OpenDefaultStream(stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate,
				framesPerBuffer, myStreamCallback2, userData);
	}
	else
	{
		streamCB = streamCallback;
		return Pa_OpenDefaultStream(stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate,
				framesPerBuffer, myStreamCallback, userData);
	}
}

PA19_API PaError CCONV PA_CloseStream(PaStream *stream) 
{
	return Pa_CloseStream(stream);
}

PA19_API PaError CCONV PA_SetStreamFinishedCallback(PaStream *stream, PaStreamFinishedCallback *streamFinishedCallback)
{
	streamFinishedCB = streamFinishedCallback;
	return Pa_SetStreamFinishedCallback(stream, myStreamFinishedCallback);
}

PA19_API PaError CCONV PA_StartStream(PaStream *stream)
{
	return Pa_StartStream(stream);
}

PA19_API PaError CCONV PA_StopStream(PaStream *stream)
{
	return Pa_StopStream(stream);
}

PA19_API PaError CCONV PA_AbortStream(PaStream *stream)
{
	return Pa_AbortStream(stream);
}
#endif//patch_11]

PA19_API PaError CCONV PA_IsStreamStopped(PaStream *stream) 
{
	return Pa_IsStreamStopped(stream);
}

PA19_API PaError CCONV PA_IsStreamActive(PaStream *stream) 
{
	return Pa_IsStreamActive(stream);
}

PA19_API const PaStreamInfo* CCONV PA_GetStreamInfo(PaStream *stream) 
{
	return Pa_GetStreamInfo(stream);
}

PA19_API PaTime CCONV PA_GetStreamTime(PaStream *stream)
{
	return Pa_GetStreamTime(stream);
}

PA19_API double CCONV PA_GetStreamCpuLoad(PaStream *stream) 
{
	return Pa_GetStreamCpuLoad(stream);
}

PA19_API PaError CCONV PA_ReadStream(PaStream *stream, void *buffer, unsigned long frames) 
{
	return Pa_ReadStream(stream, buffer, frames);
}

PA19_API PaError CCONV PA_WriteStream(PaStream *stream, const void *buffer, unsigned long frames) 
{
	return Pa_WriteStream(stream, buffer, frames);
}

PA19_API signed long CCONV PA_GetStreamReadAvailable(PaStream *stream) 
{
	return Pa_GetStreamReadAvailable(stream);
}

PA19_API signed long CCONV PA_GetStreamWriteAvailable(PaStream *stream)
{
	return Pa_GetStreamWriteAvailable(stream);
}

PA19_API PaError CCONV PA_GetSampleSize(PaSampleFormat format) 
{
	return Pa_GetSampleSize(format);
}

PA19_API void CCONV PA_Sleep(long msec)
{
	Pa_Sleep(msec);
}

#ifdef __cplusplus
}
#endif
