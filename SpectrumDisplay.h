#ifndef SPECTRUM_DISPLAY
#define SPECTRUM_DISPLAY

#include "defs.h"
#include "WindowedFftHandler.h"
#include "LineDisplayHandler2D.h"
#include <math.h>
#include <cstring>
#include <iostream>

//Class which can calculate the magnitude spectrum of externally stored audio buffers using various zero padding and window parameters, linear or dB magnitude scaling, and a linear or log frequency axis, and plot the result scaled to fit a specified size display window at a specified location, with specified clipping limits on the magnitude plot Y axis.

class SpectrumDisplay : public WindowedFftHandler, public LineDisplayHandler2D
{
public:
	
	//Constructors take A WHOLE LOTTA STUFF: FftHandler parameters, unique spectrumDisplay parameters, then displayHandler parameters, then waterfall mode parameters (spectrumDisplay unique, but optional).
	
	//This constructor allows a user specified window array.
	SpectrumDisplay(long N_In, long M_In, SAMPLE* windowIn, char* axisTypeIn, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minFreqDisplayedIn, SAMPLE maxFreqDisplayedIn,SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn);
	
	//This constructor uses a window specified by name
	SpectrumDisplay(long N_In, long M_In, char* windowNameIn, char* axisTypeIn, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minFreqDisplayedIn, SAMPLE maxFreqDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn);
	
	//Destructor
	~SpectrumDisplay();
	
	//Updates the display using the data in the buffer.
	void generateDisplayFromBuffer();
	
	
	//General Spectrum Display Set / Get fns --------------------
	
	void setM(long M_In);		//Note: A lot has to be reset when changing M. A call to generateDisplayFromBuffer() will have to be made after changing M before the displayValues are used to display.
	
	void setAxisType(char* axisTypeIn);
	void copyOutAxisType(char* outString) { strcpy(outString, axisType); };

	//Public buffer. This is public so that audio buffers which can be copied from will be easily accessible.
	//							 This buffer will never store unique audio data, just copies of other buffers which we wish to
	//								 form a spectral display from. Thus it's OK to be public.
	
	SAMPLE* realValuedInputBuffer;
//	SAMPLE* magnitudeValues;		
	
private:

	//Axis adjustment functions
	void makeXAxisDisplayLog();
	void makeXAxisDisplayLinear();
	void adjustXAxisDisplayedLimits();

	//Local arrays for display output
	SAMPLE* magnitudeValues;
	SAMPLE* xAxisValues;
	
	char* axisType;
	
};

#endif