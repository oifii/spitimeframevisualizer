/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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