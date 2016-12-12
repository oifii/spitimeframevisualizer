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