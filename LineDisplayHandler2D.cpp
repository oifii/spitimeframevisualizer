#include "LineDisplayHandler2D.h"

//Constructor
LineDisplayHandler2D :: LineDisplayHandler2D(float displayMinXIn, float displayWidthXIn, float displayMinYIn, float displayHeightYIn, SAMPLE minDataXValueDisplayedIn, SAMPLE maxDataXValueDisplayedIn, SAMPLE dataYValueLimitLowIn, SAMPLE dataYValueLimitHighIn, bool dataXUpdatingIn, bool dataYUpdatingIn, long lengthOfDataBuffersIn, const SAMPLE* dataXValuesIn, const SAMPLE* dataYValuesIn)
{
	//Assert that display dimensions have non-zero, positive width, except in dimensions which will not be plotted
	assert( displayWidthXIn > 0.0);	
	assert( displayHeightYIn > 0.0);
	
	assert( lengthOfDataBuffersIn > 0 );
	
	displayMinX = displayMinXIn;
	displayWidthX = displayWidthXIn;
	displayMinY = displayMinYIn;
	displayHeightY = displayHeightYIn;
	
	dataYValueLimitLow = dataYValueLimitLowIn;
	dataYValueLimitHigh = dataYValueLimitHighIn;	
	
	minDataXValueDisplayed = minDataXValueDisplayedIn;
	maxDataXValueDisplayed = maxDataXValueDisplayedIn;	
	
	lengthOfDataBuffers = lengthOfDataBuffersIn;
	dataXValues = dataXValuesIn;
	dataYValues = dataYValuesIn;
	
	dataXUpdating = dataXUpdatingIn;				
	dataYUpdating = dataYUpdatingIn;	
		
	displayXValues = new float[lengthOfDataBuffers];
	displayYValues = new float[lengthOfDataBuffers];
	
	//Should really be calculated based on min and max X values to be displayed and the values in the dataXValues array.
	//Currently just defaults to no zoom (plus we currently have no way of changing this since the fns arent implemented)
	currentPlotLength = lengthOfDataBuffers;	
	
	//Initialization really needs for all of data providing structure to be ready first.
	//May need to reorganize this interaction...

//		NEED TO INIALIZE DATA ARRAY IN DERIVED CLASS FIRST
/*	
	//If any dimension has been flagged as not updating, generate the display values array for that dimension. This will remain static until the display parameters are changed, and only recalculated then.
	if( !dataXUpdating )
		makeStaticDisplayArray( 0 );
		
	if( !dataYUpdating )
		makeStaticDisplayArray( 1 );
*/
}
	
	
//Destructor
LineDisplayHandler2D ::	~LineDisplayHandler2D()
{
	delete []displayXValues;
	delete []displayYValues;
}


// Change lengthOfDataBuffers - resets object displayValues
void LineDisplayHandler2D :: changeLengthOfDataBuffers( long lengthOfDataBuffersIn )
{
	assert( lengthOfDataBuffersIn > 0 );
	
	lengthOfDataBuffers = lengthOfDataBuffersIn;
	
	delete []displayXValues;
	delete []displayYValues;
	
	displayXValues = new float[lengthOfDataBuffers];
	displayYValues = new float[lengthOfDataBuffers];	
	
	//Remake static buffers
	if( !dataXUpdating )
		makeStaticDisplayArray( 0 );
		
	if( !dataYUpdating )
		makeStaticDisplayArray( 1 );
		
	//Note: refreshDisplayValues still needs to be called before displaying. This should be done in any display loop anyways though.
}
	
	
//Recalculates the values in the displayValues arrays based on the data in the dataValues arrays except for dimensions with data_Updating = flase.
void LineDisplayHandler2D :: refreshDisplayValues()
{
	//Only refresh those dimensions which have data that is updating
	
	if( dataXUpdating )
	{
		//Calculate the range of values X takes. 
		float deltaXVal = dataXValues[lengthOfDataBuffers-1] - dataXValues[0];
		
		for( int i = 0; i < lengthOfDataBuffers; i++ )
			displayXValues[i] = displayMinX + ( (dataXValues[i] - dataXValues[0]) / deltaXVal ) * displayWidthX;				
	}
	
	
	if( dataYUpdating )
	{
		for( int i = 0; i < lengthOfDataBuffers; i++ )
		{
			if( dataYValues[i] < dataYValueLimitLow )
				displayYValues[i] = displayMinY;	
			else if( dataYValues[i] > dataYValueLimitHigh )
				displayYValues[i] = displayMinY + displayHeightY;
			else
				displayYValues[i] = displayMinY + ( (dataYValues[i] - dataYValueLimitLow) / (dataYValueLimitHigh - dataYValueLimitLow) ) * displayHeightY;
		}
	}

}


//Calculates display arrays which do not need to be updated during each call to refreshDisplay().
void LineDisplayHandler2D :: makeStaticDisplayArray(int arrayNumber)
{
	if( arrayNumber == 0 )	// X dimension array. Assumes dataXValues is ordered low to high in values.
	{
		if( dataXValues == NULL )
			std::cout << "Trying to makeStaticDisplayArray for NULL X data pointer!! Nothing will be done." << std::endl;
		else
		{
			//Calculate the range of values X takes. 
			float deltaXVal = (float)(dataXValues[lengthOfDataBuffers-1] - dataXValues[0]);
			
			for( int i = 0; i < lengthOfDataBuffers; i++ )
				displayXValues[i] = displayMinX + ( (dataXValues[i] - dataXValues[0]) / deltaXVal ) * displayWidthX;				
		}
	}
	else if( arrayNumber == 1 )	// Y dimension array. NOT Assumed to be ordered.
	{
		if( dataYValues == NULL )
			std::cout << "Trying to makeStaticDisplayArray for NULL Y data pointer!! Nothing will be done." << std::endl;
		else
		{
			for( int i = 0; i < lengthOfDataBuffers; i++ )
			{
				if( dataYValues[i] < dataYValueLimitLow )
					displayYValues[i] = displayMinY;
				else if( dataYValues[i] > dataYValueLimitHigh )
					displayYValues[i] = displayMinY + displayHeightY;
				else
					displayYValues[i] = displayMinY + (dataYValues[i] - dataYValueLimitLow) * displayHeightY;
			}
		}
	}

}



void LineDisplayHandler2D :: setMinDataXValueDisplayed(SAMPLE minDataXValueDisplayedIn)
{
	
}


void LineDisplayHandler2D :: setMaxDataXValueDisplayed(SAMPLE maxDataXValueDisplayedIn)
{
	
}