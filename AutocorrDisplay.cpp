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
#include "AutocorrDisplay.h"

AutocorrDisplay :: AutocorrDisplay(long N_In, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, float minLagDisplayedIn, float maxLagDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn) : AutocorrHandler( N_In ), LineDisplayHandler2D(displayMinXIn, displayWidthXIn, displayMinYIn, displayHeightYIn, minLagDisplayedIn, maxLagDisplayedIn, dataYValueLimitLowIn, dataYValueLimitHighIn, false, true, N_In) 
{	
	//Initialize local arrays
	realValuedInputBuffer = new SAMPLE[N];
	autocorrelationValues = new SAMPLE[N];
	xAxisValues = new SAMPLE[N];
	
	//Set xAxisValues in terms of lag (samples)
	for(int i = 0; i < N; i++)
		xAxisValues[i] = i;
	
	//Set the displayHandler's data pointers now that these arrays have been initialized
	setDataXValuesPointer((const SAMPLE*) xAxisValues);
	setDataYValuesPointer((const SAMPLE*) autocorrelationValues);
	
	//Make the display array for the X axis, since it is static	
	makeStaticDisplayArray( 0 );	
}


AutocorrDisplay :: ~AutocorrDisplay()
{
	delete []realValuedInputBuffer;
	delete []autocorrelationValues;
	delete []xAxisValues;
}


void AutocorrDisplay :: generateDisplayFromBuffer()
{
	autocorr(realValuedInputBuffer, autocorrelationValues);
	refreshDisplayValues();
}