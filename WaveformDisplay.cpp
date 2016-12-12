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
#include "WaveformDisplay.h"

//Constructor
WaveformDisplay :: WaveformDisplay(long N_In, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, float minIndexDisplayedIn, float maxIndexDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn) : LineDisplayHandler2D(displayMinXIn, displayWidthXIn, displayMinYIn, displayHeightYIn, minIndexDisplayedIn, maxIndexDisplayedIn, dataYValueLimitLowIn, dataYValueLimitHighIn, false, true, N_In)
{
	assert(N_In > 0);
	
	//Initialize local arrays
	realValuedInputBuffer = new SAMPLE[N_In];
	xAxisValues = new SAMPLE[N_In];
	
	//xAxisValues are linearly spaced
	for(int i=0; i<N_In; i++)
		xAxisValues[i] = i;
		
	//Set the displayHandler's data pointers now that these arrays have been initialized
	setDataXValuesPointer((const SAMPLE*) xAxisValues);
	setDataYValuesPointer((const SAMPLE*) realValuedInputBuffer);
	
	//Make the display array for the X axis, since it is static	
	makeStaticDisplayArray( 0 );
}

	
//Destructor
WaveformDisplay ::	~WaveformDisplay()
{
	delete []realValuedInputBuffer;	
	delete []xAxisValues;
}

	
//Updates the display using the data in the buffer.
void WaveformDisplay :: generateDisplayFromBuffer()
{
	refreshDisplayValues();		
}