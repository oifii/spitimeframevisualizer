#ifndef AUTOCORR_HANDLER
#define AUTOCORR_HANDLER

#include "defs.h"
#include "FftBase.h"
#include <cmath>
#include <cassert>

#define complexMagSq(x) ( (x).real * (x).real + (x).imag * (x).imag )

//Class which calculates autocorrelation by inverting a squared magnitude FFT

class AutocorrHandler : public FftBase
{
public:

	//Constructor
	AutocorrHandler(long N_In);
	
	//Destructor
	~AutocorrHandler();
	
	virtual void setN(long N_In);
	
	void autocorr(SAMPLE* inBuffer, SAMPLE* outBuffer);	
	
protected:

	//Takes a length N input buffer and stores the N positive lag autocorrelation values in outBuffer (which can be the same as the inBuffer if desired)
	void autocorr(const SAMPLE* inBuffer, SAMPLE* outBuffer);	//version which enforces outBuffer != inBuffer		

};

#endif