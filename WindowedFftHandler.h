#ifndef WINDOWED_FFT_HANDLER
#define WINDOWED_FFT_HANDLER

#include "defs.h"
#include "FftBase.h"
#include <cassert>
#include <cstring>
#include <math.h>
#include <iostream>

#define complexMag(x) ( sqrt( (x).real * (x).real + (x).imag * (x).imag ) )

//Class which takes care of FFT's - Manages windowing and taking FFT's, returning complex values, magnitudes, or magnitudes in dB.
//
//  Currently does linear phase processing rather than zero phase.
//	Currently normalizes all FFTs to be unitary
//
//  Input buffers can be odd lengths (as can accompanying window functions),
//		but the final FFT length currently MUST be a power of 2.


class WindowedFftHandler : public FftBase
{
public:

	//Constructor specifying window by an externally constructed array
	WindowedFftHandler(long N_In, long M_In, SAMPLE* windowIn = NULL);
	
	//Constructor specifying window by name. Window samples will be calculated by WindowedFftHandler.
	WindowedFftHandler(long N_In, long M_In, char* windowNameIn);
	
	~WindowedFftHandler();
	
	
	//Input and Fft parameter setting functions
	void setN(long N_In);		//Will generate a new window of the current type with length N_In if using a name-specified window
	void setN(long N_In, SAMPLE* windowIn);	//Allows a new user-defined window to be supplied with length N_In. Mandatory for non name-specified windows
	virtual	void setM(long M_In);		//Inheriting classes may have to do more adjustments when changing M => virtual fn.
	
	//Window set / get functions
	void setWindow(SAMPLE* windowIn) { memcpy(window, windowIn, N * sizeof(SAMPLE)); };
	void setWindow(char* windowName);
	void copyOutWindow(SAMPLE* outDestination) { memcpy(outDestination, window, N * sizeof(SAMPLE)); };
	void copyOutWindowName(char* outString) { strcpy(outString, windowName); };
	long getWindowLength() { return N; };
	
	
protected:
		
	//Takes an fft of N real values from the inputBuffer using zero padding to length M and stores the M complex valued positive frequency results in the complex value formatted outBuffer.
	void realUnitaryFft(SAMPLE* inBuffer, complex* outBuffer);
	void realUnitaryFft(const SAMPLE* inBuffer, complex* outBuffer);	//Version which enforces outBuffer != inBuffer
	
	//Takes an fft of N real values from the inputBuffer using zero padding to length M, calculates the fft bin magnitudes and stores the M results in the output buffer (which can be the input buffer if there is no zero padding and N = M).
	void findUnitaryFftMagnitudes(SAMPLE* inBuffer, SAMPLE* outBuffer);
	void findUnitaryFftMagnitudes(const SAMPLE* inBuffer, SAMPLE* outBuffer);	//Version which enforces outBuffer != inBuffer
	
	//Takes an fft of N real values from the inputBuffer using zero padding to length M, calculates the fft bin magnitudes in dB and stores the M results in the output buffer (which can be the input buffer if there is no zero padding and N = M).
	void findUnitaryFftMagnitudesdB(SAMPLE* inBuffer, SAMPLE* outBuffer);
	void findUnitaryFftMagnitudesdB(const SAMPLE* inBuffer, SAMPLE* outBuffer); //Version which enforces outBuffer != inBuffer
		

private:

	//Window generation fns (Currently all linear phase, not zero phase)
	void setWindowToRectangular();
	void setWindowToHann();
	void setWindowToHamming();
	void setWindowToBlackman();
	
	bool useRectangular;
		
	SAMPLE* window;	//Note: window must be N samples long
	char* windowName;
	
};
#endif