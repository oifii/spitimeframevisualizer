#ifndef LINE_DISPLAY_HANDLER2D
#define LINE_DISPLAY_HANDLER2D

#include "defs.h"
#include <cassert>
#include <memory.h>
#include <iostream>

//Abstract class which handles translation between (x,y) (SAMPLE) data and (float) display coordinates given a particular display location and size
//  as well as a few data display formatting parameters. Must be inherited by a "controller" class which forms a complete logical unit
//	of data acquisition & display which implements the generateDisplayFromBuffer() fn.
//
// NOTE: This class assumes X axis data is ordered in ascending (low to high) order. Y values can be unordered and are constrained
//					to lie within a specified range which is then mapped to the specified display area, clipping Y values which would have been
//					outside the min and max values. The X data values are mapped to fill the specified X display range using the first (minimum) and last 
//					(maximum) values.
//
// ---Make all OpenGL-ish? Include actual draw fn that pushes / pops a matrix, drawing a line segment based on the data?
//				In this case, the "plotLength" variable should be absorbed into this class since it won't be needed externally.
//
// ---Include downsampling option for when we want to display a large data input in a small window??
// ---Use 2D point struct?

class LineDisplayHandler2D
{
public:

	//Constructor. Allows data pointers to be neglected and set later. This often needs to be done since an inheriting class must initialize its
	//								arrays before these pointers can be assigned.
	LineDisplayHandler2D(float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minDataXValueDisplayedIn, SAMPLE maxDataXValueDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn, bool dataXUpdatingIn, bool dataYUpdatingIn, long lengthOfDataBuffersIn, const SAMPLE* dataXValuesIn = NULL, const SAMPLE* dataYValuesIn = NULL);
	
	//Destructor
	~LineDisplayHandler2D();
	
	//Must be implemented in inheriting "controller" to allow polymorphic updating of display values given different types of data generation
	virtual void generateDisplayFromBuffer() = 0;
	
	//Returns the number of indices to be plotted
	long getPlotLength() { return currentPlotLength; };
		
	//Display value array copy fns
	void copyDisplayXValues(float* copyDestination) { memcpy( copyDestination, displayXValues, lengthOfDataBuffers * sizeof(float) ); };
	void copyDisplayYValues(float* copyDestination) { memcpy( copyDestination, displayYValues, lengthOfDataBuffers * sizeof(float) ); };
	
	//Display value array in place access fns
	const float* getDisplayXValuesPointer() { return (const float*) displayXValues; };
	const float* getDisplayYValuesPointer() { return (const float*) displayYValues; };
	
	//Utility Fns
	void setDisplayMinX(float displayMinXIn) { displayMinX = displayMinXIn; };
	float getDisplayMinX() { return displayMinX; };
	void setDisplayWidthX(float displayWidthXIn) { assert(displayWidthXIn >= 0.0); displayWidthX = displayWidthXIn; };
	float getDisplayWidthX() { return displayWidthX; };
	
	void setDisplayMinY(float displayMinYIn) { displayMinY = displayMinYIn; };
	float getDisplayMinY() { return displayMinY; };	
	void setDisplayHeightY(float displayHeightYIn) { assert(displayHeightYIn >= 0.0); displayHeightY = displayHeightYIn; };
	float getDisplayHeightY() { return displayHeightY; };	
	
	void setDataYValueLimitLow(SAMPLE dataYValueLimitLowIn) { dataYValueLimitLow = dataYValueLimitLowIn; };
	SAMPLE getDataYValueLimitLow() { return dataYValueLimitLow; };
	void setDataYValueLimitHigh(SAMPLE dataYValueLimitHighIn) { dataYValueLimitHigh = dataYValueLimitHighIn; };
	SAMPLE getDataYValueLimitHigh() { return dataYValueLimitHigh; };	
	
	void setMinDataXValueDisplayed(SAMPLE minDataXValueDisplayedIn);
	SAMPLE getMinDataXValueDisplayed() { return minDataXValueDisplayed; };	
	void setMaxDataXValueDisplayed(SAMPLE maxDataXValueDisplayedIn);
	SAMPLE getMaxDataXValueeDisplayed() { return maxDataXValueDisplayed; };

	
protected:

	//Change length of data buffers - resets object displayValues
	void changeLengthOfDataBuffers( long lengthOfDataBuffersIn );

	//Set the data pointers post-initialization
	void setDataXValuesPointer( const SAMPLE* dataXValuesIn ) { dataXValues = dataXValuesIn; };
	void setDataYValuesPointer( const SAMPLE* dataYValuesIn ) { dataYValues = dataYValuesIn; };
	
	//Recalculates the values in the displayValues arrays based on the data in the dataValues arrays except for dimensions with data_Updating = flase.
	void refreshDisplayValues();

	//Calculates display arrays which do not need to be updated during each call to refreshDisplay().
	void makeStaticDisplayArray(int arrayNumber);
	
private:

	//Display area parameters
	float displayMinX;
	float displayWidthX;
	float displayMinY;
	float displayHeightY;
	
	//Y axis data display formatting
	SAMPLE dataYValueLimitLow;		//Data value which maps to displayMinY. Lower data values will be clipped to this height
	SAMPLE dataYValueLimitHigh;		//Data value which maps to displayMinY + displayWidthY. Higher data values will be clipped to this height
	
	//Pointers to data to be displayed
	long lengthOfDataBuffers;	
	const SAMPLE* dataXValues;
	const SAMPLE* dataYValues;
	
	//Booleans which mark the dimensions of the data arrays which will be changing during updates
	bool dataXUpdating;
	bool dataYUpdating;
	
	//Keep track of the range of x axis values we are currently displaying
	SAMPLE minDataXValueDisplayed;
	SAMPLE maxDataXValueDisplayed;
	int startDataXIndex;
	int endDataXIndex;
	int currentPlotLength;			//Keeps track of how many indices currently need to be plotted

	
	//Arrays which hold the calculated display output
	float* displayXValues;
	float* displayYValues;
};

#endif