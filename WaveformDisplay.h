#ifndef WAVEFORM_DISPLAY
#define WAVEFORM_DISPLAY

#include "defs.h"
#include "LineDisplayHandler2D.h"
#include <cassert>

//Class that displays a waveform but does no computation

class WaveformDisplay : public LineDisplayHandler2D
{
public:
	
	//Constructor
	WaveformDisplay(long N_In, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, float minIndexDisplayedIn, float maxIndexDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn);
	
	//Destructor
	~WaveformDisplay();
	
	//Updates the display using the data in the buffer.
	void generateDisplayFromBuffer();

	//Public buffer. This is public so that audio buffers which can be copied from will be easily accessible.
	//							 This buffer will never store unique audio data, just copies of other buffers which we wish to
	//								 form a display from. Thus it's OK to be public.
	
	SAMPLE* realValuedInputBuffer;
	
private:

	SAMPLE* xAxisValues;
	
};

#endif