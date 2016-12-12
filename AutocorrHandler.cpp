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
#include "AutocorrHandler.h"

AutocorrHandler :: AutocorrHandler(long N_In) : FftBase(N_In, 2*N_In) {}


AutocorrHandler :: ~AutocorrHandler() {}


void AutocorrHandler :: setN(long N_In)
{
	FftBase::setM(2 * N_In);
	FftBase::setN(N_In);	
}


//Takes a length N input buffer and stores the N positive lag autocorrelation values in outBuffer (which can be the same as the inBuffer if desired)
void AutocorrHandler :: autocorr(SAMPLE* inBuffer, SAMPLE* outBuffer)
{
		//Copy input buffer into imaginary values and zero pad, then copy from here into the real values where they should be, in bit reversed order (including the zero padding).
	for( int i = 0; i < N; i++ )
		tempBuffer[i].imag = inBuffer[i];
		
	for( int i = N; i < M; i++ )
		tempBuffer[i].imag = 0.0;
	
	for( int i = 0; i < M; i++ )
		tempBuffer[i].real = tempBuffer[bitReverseLookupTable[i]].imag;
		
	//Zero out the imaginary values we either used temporarily with zero padding or haven't yet cleared this function call (if not using zero padding, there are only N imaginary values to set to 0 anyways).
	for( int i = 0; i < N; i++ )
		tempBuffer[i].imag = 0.0;		
	
	//----------------------------
	//take fft
	fft( tempBuffer );
	//----------------------------	
	
	//Place the squared magnitude in the imaginary part of the tempBuffer so we can bitReverse copy out of it into the real part
	for( int i = 0; i < M; i++ )
		tempBuffer[i].imag = complexMagSq(tempBuffer[i]);
	
	//Bit reverse the squared magnitude values from the imaginary part to the real part
	for( int i = 0; i < M; i++ )
		tempBuffer[i].real = tempBuffer[bitReverseLookupTable[i]].imag;	

	//Zero out the imaginary part
	for( int i = 0; i < M; i++ )
		tempBuffer[i].imag = 0.0;
	
	//----------------------------	
	//take "inverse" fft - Note: Using fft for ifft reverses time domain signal, but autocorrelation is symmetric so it doesn't matter
	fft( tempBuffer );
	//----------------------------	
	
	//copy first half (0 and positive lags) into outBuffer, scaling for unitary FFT
	for( int i = 0; i < N; i++ )
		outBuffer[i] = 0.5 * tempBuffer[i].real / N;		//Need *0.5 scaling for autocorrValues[0] = sum of squares of input signal ... ?
}


//version which enforces outBuffer != inBuffer
void AutocorrHandler :: autocorr(const SAMPLE* inBuffer, SAMPLE* outBuffer)
{
	//Assert that we will not write the results into the same buffer we read from, then process as usual
	assert( inBuffer != outBuffer );
	autocorr((SAMPLE*) inBuffer, outBuffer);
}