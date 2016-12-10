#include "SpectrumDisplay.h"

//Constructor which allows a user specified window array.
SpectrumDisplay :: SpectrumDisplay(long N_In, long M_In, SAMPLE* windowIn, char* axisTypeIn, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minFreqDisplayedIn, SAMPLE maxFreqDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn) : WindowedFftHandler(N_In, M_In, windowIn), LineDisplayHandler2D( displayMinXIn, displayWidthXIn, displayMinYIn, displayHeightYIn, minFreqDisplayedIn, maxFreqDisplayedIn, dataYValueLimitLowIn, dataYValueLimitHighIn, false, true, ( M_In / 2))
{	
	
	axisType = new char[16];
	
	//Initialize local arrays
	realValuedInputBuffer = new SAMPLE[N_In];
	magnitudeValues = new SAMPLE[M/2];
	xAxisValues = new SAMPLE[M/2];
	
	//Set the displayHandler's data pointers now that these arrays have been initialized
	setDataXValuesPointer((const SAMPLE*) xAxisValues);
	setDataYValuesPointer((const SAMPLE*) magnitudeValues);

	//Fill xAxisValues with desired scale such that magnitude values correspond to linear or log spacing on the x axis.
	if( strcmp(axisType, "log") || strcmp(axisType, "Log") )
	{
		axisType = "Log";
		makeXAxisDisplayLog();
	}
	else if( strcmp(axisType, "linear") || strcmp(axisType, "Linear") )
	{
		axisType = "Linear";
		makeXAxisDisplayLinear();
	}
	else
	{
		std::cout << "Invalid axisType specified!! axisType will be set to \"INVALID\"" << std::endl;
		axisType = "INVALID";
	}
	
	//Make the display array for the X axis, since it is static	
	makeStaticDisplayArray( 0 );
}


//Constructor which specifies window by name.
SpectrumDisplay :: SpectrumDisplay(long N_In, long M_In, char* windowNameIn, char* axisTypeIn, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minFreqDisplayedIn, SAMPLE maxFreqDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn) : WindowedFftHandler(N_In, M_In, windowNameIn), LineDisplayHandler2D( displayMinXIn, displayWidthXIn, displayMinYIn, displayHeightYIn, minFreqDisplayedIn, maxFreqDisplayedIn, dataYValueLimitLowIn, dataYValueLimitHighIn, false, true, ( M_In / 2))
{
		
	axisType = new char[16];
	axisType = axisTypeIn;
	
	//Initialize local arrays
	realValuedInputBuffer = new SAMPLE[N];	
	magnitudeValues = new SAMPLE[M/2];
	xAxisValues = new SAMPLE[M/2];
	
	//Set the displayHandler's data pointers now that these arrays have been initialized
	setDataXValuesPointer((const SAMPLE*) xAxisValues);
	setDataYValuesPointer((const SAMPLE*) magnitudeValues);
	
	//Fill xAxisValues with desired scale such that magnitude values correspond to linear or log spacing on the x axis.
	if( strcmp(axisTypeIn, "log") == 0 || strcmp(axisTypeIn, "Log") == 0 )
	{
		axisType = "Log";
		makeXAxisDisplayLog();
	}
	else if( strcmp(axisTypeIn, "linear") == 0 || strcmp(axisTypeIn, "Linear") == 0 )
	{
		axisType = "Linear";
		makeXAxisDisplayLinear();
	}
	else
	{
		std::cout << "Invalid axisType specified!! axisType will be set to \"INVALID\"" << std::endl;
		axisType = "INVALID";
	}
	
	//Make the display array for the X axis, since it is static	
	makeStaticDisplayArray( 0 );
}


SpectrumDisplay :: ~SpectrumDisplay()
{
	delete []realValuedInputBuffer;
	delete []magnitudeValues;
	delete []xAxisValues;
	delete []axisType;
}


void SpectrumDisplay :: generateDisplayFromBuffer()
{
	findUnitaryFftMagnitudesdB( realValuedInputBuffer, magnitudeValues );
	refreshDisplayValues();
}


void SpectrumDisplay :: setM(long M_In)
{
	this->WindowedFftHandler::setM(M_In);			//set in WindowedFftHandler. Will assert that M_In is valid
	
	//Then take care of extra SpectrumDisplay stuff ---------------------
	delete []magnitudeValues;
	delete []xAxisValues;
	
	magnitudeValues = new SAMPLE[M/2];
	xAxisValues = new SAMPLE[M/2];
	
	//Regenerate xAxisValues
	if( strcmp(axisType, "Log") == 0 )
		makeXAxisDisplayLog();
	else if( strcmp(axisType, "Linear") == 0 )
		makeXAxisDisplayLinear();
	
	//Reset LineDisplayHandler2D's data pointers now that arrays have been re-initialized
	setDataXValuesPointer((const SAMPLE*) xAxisValues);
	setDataYValuesPointer((const SAMPLE*) magnitudeValues);
	
	//set new M in LineDisplayHandler2D now that SpectrumDisplay local arrays have been re-initialized and LineDisplayHandler2D's data pointers have been updated
	changeLengthOfDataBuffers(M_In);	
	
	//Object is now ready for a call to generateDisplayFromBuffer. displayValues still need to be set for "updating" dimensions.
}


void SpectrumDisplay :: setAxisType(char* axisTypeIn)
{
	if( strcmp(axisTypeIn, "log") == 0 || strcmp(axisTypeIn, "Log") == 0)
	{
		axisType = "Log";
		makeXAxisDisplayLog();
	}
	else if( strcmp(axisTypeIn, "linear") == 0 || strcmp(axisTypeIn, "Linear") == 0)
	{
		axisType = "Linear";
		makeXAxisDisplayLinear();
	}
	else
		std::cout << "Invalid axisType specified!! axisType will not be changed." << std::endl;
	
	//Recalculate display points for new X axis
	makeStaticDisplayArray( 0 );
}


void SpectrumDisplay :: makeXAxisDisplayLog()
{
		xAxisValues[0] = 0.0;
	for(int binNum = 1; binNum < M/2; binNum++)
//		xAxisValues[binNum] = log10( binNum * (SRATE / (2.0 * getM())) );
		xAxisValues[binNum] = pow( binNum * (SRATE / (2.0 * getM())) , 0.3);
}


void SpectrumDisplay :: makeXAxisDisplayLinear()
{
	for(int binNum = 0; binNum < M/2; binNum++)
		xAxisValues[binNum] = binNum * ((double)SRATE / getM());
}