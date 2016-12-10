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