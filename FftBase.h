#ifndef FFT_BASE
#define FFT_BASE

#include "defs.h"
#include <cassert>
#include <math.h>

typedef struct { float real ; float imag ; } complex;

//Abstract? class which forms the base of any class which uses FFTs to do what it does

class FftBase
{
public:

	//Constructor
	FftBase(long N_In, long M_In);
	
	//Destructor
	~FftBase();
	
	virtual void setN(long N_In);		//Inheriting classes may need to do more adjustments when changing N => virtual fn.
	virtual void setM(long M_In);		//Inheriting classes may need to do more adjustments when changing M => virtual fn.
	long getN() { return N; }
	long getM() { return M; }

protected:

	//Fft fns
	void fft(complex* fftBuffer);
//	void cfft(complex* zeroPaddedInBuffer, SAMPLE* outBuffer);
	
	//Other utilities
//	void copyNRealInputToMLongTempBufferBitReversed(SAMPLE* inBuffer);	//Copies N real values from input buffer to M long tempBuffer in bitReversed order
	void makeTrigTables(SAMPLE* inBufferSin, SAMPLE* inBufferCos);
	void makeBitReverseTable(int* inBitReverseLookup);
	
	//Fft trig and reverse lookup tables
	SAMPLE* sinLookupTable;
	SAMPLE* cosLookupTable;
	int* bitReverseLookupTable;
	
	complex* tempBuffer;
	
	long N;	//length of input buffer signal
	long M;	//length of fft.
	
};

#endif