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