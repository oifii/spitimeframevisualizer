#ifndef AUTOCORR_DISPLAY
#define AUTOCORR_DISPLAY

#include "defs.h"
#include "AutocorrHandler.h"
#include "LineDisplayHandler2D.h"

class AutocorrDisplay : public AutocorrHandler, public LineDisplayHandler2D
{
public:
	
	//Constructor
	AutocorrDisplay(long N_In, float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, float minLagDisplayedIn, float maxLagDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn);
	
	//Destructor
	~AutocorrDisplay();
	
	//Updates the display using the data in the buffer.
	void generateDisplayFromBuffer();

	//Public buffer. This is public so that audio buffers which can be copied from will be easily accessible.
	//							 This buffer will never store unique audio data, just copies of other buffers which we wish to
	//								 form an autocorrelation display from. Thus it's OK to be public.
	
	SAMPLE* realValuedInputBuffer;
		
private:
	
	SAMPLE* autocorrelationValues;		
	SAMPLE* xAxisValues;
};

#endif