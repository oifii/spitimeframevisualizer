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
#include "FftBase.h"

//Constructor
FftBase :: FftBase(long N_In, long M_In)
{
	assert( M_In >= N_In );
	assert( N_In > 0 );
	
	N = N_In;
	M = M_In;
	
	tempBuffer = new complex[M];
	
	//initialize lookup tables
	sinLookupTable = new SAMPLE[M];
	cosLookupTable = new SAMPLE[M];
	makeTrigTables( sinLookupTable, cosLookupTable );
	
	bitReverseLookupTable = new int[M];
	makeBitReverseTable( bitReverseLookupTable );
}


FftBase :: ~FftBase()
{
	delete []tempBuffer;
	delete []sinLookupTable;
	delete []cosLookupTable;
	delete []bitReverseLookupTable;
}


void FftBase :: setN(long N_In)
{
	assert( M >= N );	//Assert that the new input signal length is less than the total fft length	
	N = N_In;
}
	

void FftBase :: setM(long M_In )
{
	assert( M_In >= N );
	
	M = M_In;
	
	delete []sinLookupTable;
	delete []cosLookupTable;
	delete []bitReverseLookupTable;
	
	sinLookupTable = new SAMPLE[M];
	cosLookupTable = new SAMPLE[M];
	makeTrigTables( sinLookupTable, cosLookupTable );
	
	bitReverseLookupTable = new int[M];
	makeBitReverseTable( bitReverseLookupTable );
	
	delete []tempBuffer;
	tempBuffer = new complex[M];
}


void FftBase :: fft(complex* fftBuffer)
{
  // performs an in-place fft on the complex data in the inBuffer
  // bit reversing should already have been done.
  for (int halfSize = 1; halfSize < M; halfSize *= 2)
  {  
     SAMPLE phaseShiftStepR = cosLookupTable[halfSize];
     SAMPLE phaseShiftStepI = sinLookupTable[halfSize];

     // current phase shift
     SAMPLE currentPhaseShiftR = 1.0;
     SAMPLE currentPhaseShiftI = 0.0;

     for (int fftStep = 0; fftStep < halfSize; fftStep++)
     {
       for (int i = fftStep; i < M; i += 2 * halfSize)
       {
         int offset = i + halfSize;
          
         SAMPLE tr = (currentPhaseShiftR * fftBuffer[offset].real) - (currentPhaseShiftI * fftBuffer[offset].imag);
         SAMPLE ti = (currentPhaseShiftR * fftBuffer[offset].imag) + (currentPhaseShiftI * fftBuffer[offset].real);

         fftBuffer[offset].real = fftBuffer[i].real - tr;
         fftBuffer[offset].imag = fftBuffer[i].imag - ti;

         fftBuffer[i].real += tr;
         fftBuffer[i].imag += ti;
       }

       SAMPLE tmpR = currentPhaseShiftR;

       currentPhaseShiftR = (tmpR * phaseShiftStepR) - (currentPhaseShiftI * phaseShiftStepI);
       currentPhaseShiftI = (tmpR * phaseShiftStepI) + (currentPhaseShiftI * phaseShiftStepR);
     }

  }
}

/*
void FftBase :: copyNRealInputToMLongTempBufferBitReversed(SAMPLE* inBuffer)
{
	
}
*/

//Builds trig fn lookup tables for size M Fft
void FftBase :: makeTrigTables(SAMPLE* inBufferSin, SAMPLE* inBufferCos)
{	
	//0 index not used, set to 0 to be safe
	inBufferSin[0] = 0;
	inBufferCos[0] = 0;
	
	for( int i = 1; i < M; i++ )
	{
		inBufferSin[i] = sin( -MY_PI / i );
		inBufferCos[i] = cos( -MY_PI / i );
	}
}



//Builds a bit reverse lookup table for a length M Fft where bitReverseLookupTable[i] = (i, bit reversed)
void FftBase :: makeBitReverseTable(int* inBitReverseLookupTable)
{
	inBitReverseLookupTable[0] = 0;
	
	for (int limit = 1, bit = M / 2; limit < M; limit <<= 1, bit >>= 1)
    for (int i = 0; i < limit; i++)
      inBitReverseLookupTable[i + limit] = inBitReverseLookupTable[i] + bit;
}