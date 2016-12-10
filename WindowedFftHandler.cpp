#include "WindowedFftHandler.h"

//Constructor specifying window by an externally constructed array.
WindowedFftHandler :: WindowedFftHandler(long N_In, long M_In, SAMPLE* windowIn) : FftBase(N_In, M_In)
{
	assert(windowIn != NULL);
	
	windowName = new char[16];
	windowName = "User Defined";		
	
	//initialize window and copy in user defined data
	window = new SAMPLE[N];
	memcpy(window, windowIn, N * sizeof(SAMPLE));
}



//Constructor specifying window by name. Window samples will be calculated by WindowedFftHandler.
WindowedFftHandler :: WindowedFftHandler(long N_In, long M_In, char* windowNameIn) : FftBase(N_In, M_In)
{
	windowName = new char[16];
	window = NULL;	//Default window to NULL. Memory will be allocated if window other than rectangular is specified
	
	setWindow(windowNameIn);
}

	
	
WindowedFftHandler :: ~WindowedFftHandler()
{
	if( window != NULL)
		delete []window;		
		
	delete []windowName;
}

		
		
//Takes an fft of N (N) real values from the inputBuffer using zero padding to length M (M) and stores the M complex valued positive frequency results in the complex value formatted outBuffer.
void WindowedFftHandler :: realUnitaryFft(SAMPLE* inBuffer, complex* outBuffer)
{
	if( M > N )
	{
		//Copy input buffer into imaginary values and zero pad, then copy from here into the real values where they should be, in bit reversed order (including the zero padding).
		if( useRectangular )
			for( int i = 0; i < N; i++ )
				outBuffer[i].imag = inBuffer[i] / N;		//Use normalization factor (for a rectangular window with unity DC gain);			
		else
			for( int i = 0; i < N; i++ )
				outBuffer[i].imag = inBuffer[i] * window[i];
			
		for( int i = N; i < M; i++ )
			outBuffer[i].imag = 0.0;

		//SAMPLE normalizationFactor = sqrt(N);		//Normalize for unitary DFT		
		SAMPLE normalizationFactor = sqrt((double)N); //spi		
		for( int i = 0; i < M; i++ )
			outBuffer[i].real = tempBuffer[bitReverseLookupTable[i]].imag / normalizationFactor;
	}
	else	//We aren't using zero padding - just bit reverse the order directly from the input buffer
	{
		if( useRectangular )
		{
			//SAMPLE normalizationFactor = sqrt(N) * N;		//Normalize for unitary DFT and rectangular window with unity DC gain
			SAMPLE normalizationFactor = sqrt((double)N) * N; //spi
			for( int i = 0; i < M; i++ )
				outBuffer[i].real = inBuffer[bitReverseLookupTable[i]] / normalizationFactor;
		}
		else
		{
			for( int i = 0; i < M; i++ )
				outBuffer[i].real = inBuffer[bitReverseLookupTable[i]] * window[bitReverseLookupTable[i]];	
		}
	}

	//Zero out the imaginary values we either used temporarily with zero padding or haven't yet cleared this function call (if not using zero padding, there are only N imaginary values to set to 0 anyways).
	for( int i = 0; i < N; i++ )
		outBuffer[i].imag = 0.0;		
  
  //Compute fft
	fft( outBuffer );
		
}



//realFft that enforces inBuffer != outBuffer (ensures inBuffer values will not be changed)
void WindowedFftHandler :: realUnitaryFft(const SAMPLE* inBuffer, complex* outBuffer)
{
	//Assert that we will not write the results into the same buffer we read from, then process as usual
	assert( inBuffer != (SAMPLE*) outBuffer );
	realUnitaryFft( (SAMPLE*) inBuffer, outBuffer);
}


	
//Takes an fft of N (N) real values from the inputBuffer using zero padding to length M (M), calculates the fft bin magnitudes and stores the M results in the output buffer (which can be the input buffer if there is no zero padding and N = M).
void WindowedFftHandler :: findUnitaryFftMagnitudes(SAMPLE* inBuffer, SAMPLE* outBuffer)
{
	if( M > N )
	{
		//Copy input buffer into imaginary values and zero pad, then copy from here into the real values where they should be, in bit reversed order (including the zero padding).
		if( useRectangular )
			for( int i = 0; i < N; i++ )
				tempBuffer[i].imag = inBuffer[i] / N;		//Use normalization factor (for a rectangular window with unity DC gain);			
		else
			for( int i = 0; i < N; i++ )
				tempBuffer[i].imag = inBuffer[i] * window[i];
			
		for( int i = N; i < M; i++ )
			tempBuffer[i].imag = 0.0;
		
		for( int i = 0; i < M; i++ )
			tempBuffer[i].real = tempBuffer[bitReverseLookupTable[i]].imag;
	}
	else	//We aren't using zero padding - just bit reverse the order directly from the input buffer
	{
		if( useRectangular )
			for( int i = 0; i < M; i++ )
				tempBuffer[i].real = inBuffer[bitReverseLookupTable[i]] / N; //Use normalization factor (for a rectangular window with unity DC gain)
		else
			for( int i = 0; i < M; i++ )
				tempBuffer[i].real = inBuffer[bitReverseLookupTable[i]] * window[bitReverseLookupTable[i]];
	}

	//Zero out the imaginary values we either used temporarily with zero padding or haven't yet cleared this function call (if not using zero padding, there are only N imaginary values to set to 0 anyways).
	for( int i = 0; i < N; i++ )
		tempBuffer[i].imag = 0.0;		
	
  //Compute fft since we now have our real input stored as the .real values in bit-reversed order and all the .imag values set to 0.0.
	fft( tempBuffer );
	
	//Compute magnitudes and store into outBuffer
	for( int i = 0; i < M; i++ )
		outBuffer[i] = complexMag(tempBuffer[i]);	
			
}



//findFftMagnitudes that enforces inBuffer != outBuffer (ensures inBuffer values will not be changed)
void WindowedFftHandler :: findUnitaryFftMagnitudes(const SAMPLE* inBuffer, SAMPLE* outBuffer)
{
	//Assert that we will not write the results into the same buffer we read from, then process as usual
	assert( inBuffer != outBuffer );	
	findUnitaryFftMagnitudes( (SAMPLE*) inBuffer, outBuffer);
}



//Takes an fft of N (N) real values from the inputBuffer using zero padding to length M (M), calculates the fft bin magnitudes in dB and stores the M results in the output buffer (which can be the input buffer if there is no zero padding and N = M).
//This function is the same as findFftMagnitudes except it converts to dB at the end when copying into the outBuffer
void WindowedFftHandler :: findUnitaryFftMagnitudesdB(SAMPLE* inBuffer, SAMPLE* outBuffer)
{	
	if( M > N )
	{
		//Copy input buffer into imaginary values and zero pad, then copy from here into the real values where they should be, in bit reversed order (including the zero padding).
		if( useRectangular )
			for( int i = 0; i < N; i++ )
				tempBuffer[i].imag = inBuffer[i] / N;		//Use normalization factor (for a rectangular window with unity DC gain)
		else
			for( int i = 0; i < N; i++ )
				tempBuffer[i].imag = inBuffer[i] * window[i];
			
		for( int i = N; i < M; i++ )
			tempBuffer[i].imag = 0.0;
		
		for( int i = 0; i < M; i++ )
			tempBuffer[i].real = tempBuffer[bitReverseLookupTable[i]].imag;
	}
	else
	{
		//We aren't using zero padding - just bit reverse the order directly from the input buffer
		if( useRectangular )
			for( int i = 0; i < M; i++ )
				tempBuffer[i].real = inBuffer[bitReverseLookupTable[i]] / N; //Use normalization factor (for a rectangular window with unity DC gain)
		else
			for( int i = 0; i < M; i++ )
				tempBuffer[i].real = inBuffer[bitReverseLookupTable[i]] * window[bitReverseLookupTable[i]];		
	}

	//Zero out the imaginary values we either used temporarily with zero padding or haven't yet cleared this function call (if not using zero padding, there are only N imaginary values to set to 0 anyways).
	for( int i = 0; i < N; i++ )
		tempBuffer[i].imag = 0.0;		
	
  //Compute fft since we now have our real input stored as the .real values in bit-reversed order and all the .imag values set to 0.0.
	fft( tempBuffer );
	
	//Compute dB magnitudes and store into outBuffer
	//SAMPLE normalizationFactor = 10.0 * log10(N);
	SAMPLE normalizationFactor = 10.0 * log10((double)N); //spi
	for( int i = 0; i < M; i++ )
		outBuffer[i] = 20.0 * log10( complexMag(tempBuffer[i]) ) + normalizationFactor; 
}



//findFftMagnitudesdB that enforces inBuffer != outBuffer (ensures inBuffer values will not be changed)
void WindowedFftHandler :: findUnitaryFftMagnitudesdB(const SAMPLE* inBuffer, SAMPLE* outBuffer)
{
	//Assert that we will not write the results into the same buffer we read from, then process as usual
	assert( inBuffer != outBuffer );
	findUnitaryFftMagnitudesdB( (SAMPLE*) inBuffer, outBuffer);
}



//Sets the window by string name	
void WindowedFftHandler :: setWindow(char* windowNameIn)
{
	if( strcmp(windowNameIn, "Rectangular") == 0 || strcmp(windowNameIn, "rectangular") == 0 || strcmp(windowNameIn, "Rect") == 0 || strcmp(windowNameIn, "rect") == 0 )
	{
		windowName = "Rectangular";
		setWindowToRectangular();
	}
	else if( strcmp(windowNameIn, "Hann") == 0 || strcmp(windowNameIn, "hann") == 0 || strcmp(windowNameIn, "Hanning") == 0 || strcmp(windowNameIn, "hanning") == 0 )
	{
		windowName = "Hann";
		setWindowToHann();	
	}
	else if( strcmp(windowNameIn, "Hamming") == 0 || strcmp(windowNameIn, "hamming") == 0 )
	{
		windowName = "Hamming";
		setWindowToHamming();
	}
	else if( strcmp(windowNameIn, "Blackman") == 0 || strcmp(windowNameIn, "blackman") == 0 )
	{
		windowName = "Blackman";
		setWindowToBlackman();
	}
	else
	{
		std::cout << "WARNING: Improper window name requested during WindowedFftHandler.setWindow()! Window has been left unchanged." << std::endl;
		assert(0);
	}
}


void WindowedFftHandler :: setM(long M_In)
{
	FftBase::setM(M_In);	
}

void WindowedFftHandler :: setN(long N_In)
{
	FftBase::setN(N_In);

	assert( !strcmp(windowName, "userDefined") );	//A new user defined window must be supplied if one was supplied previously and we are changing N
	
	if( window != NULL)
	{
		delete []window;
		window = NULL;
	}
	
	//Rectangular window requires no change - window will stay NULL, useRectangular will stay true
	//Otherwise, window setting functions below will allocate new memory for the length N window
	if( strcmp(windowName, "Hann") == 0 )
		setWindowToHann();
	else if ( strcmp(windowName, "Hamming") == 0 )
		setWindowToHamming();
	else if ( strcmp(windowName, "Blackman") == 0 )
		setWindowToBlackman();	
}



void WindowedFftHandler :: setN(long N_In, SAMPLE* windowIn)
{
	FftBase::setN(N_In);
	
	if( window != NULL)
		delete []window;

	//Allocate new memory for length N window		
	window = new SAMPLE[N];
	
	//Set the window to be the user defined, supplied window
	windowName = "User Defined";
	memcpy(window, windowIn, N * sizeof(SAMPLE));

}


//Window generation functions -----------
//All functions currently generate symmetric linear phase windows
//Note that omega_M below refers to the radians / sample sampling rate of a length M DFT: 2 * PI / M.

//Rectangular: ~ -13 dB sidelobes, -6 dB / octave roll off, 2 omega_(M) wide main lobe
void WindowedFftHandler :: setWindowToRectangular()
{
	useRectangular = true;
}


//Hann / Hanning: ~ -32 dB sidelobes, -12 dB / octave roll off, 4 omega_(M-2) wide main lobe
//	 								This implementation includes zeros at the endpoints, making it have non-zero length M-2.
void WindowedFftHandler :: setWindowToHann()
{
	//Allocate memory if this is being called from the constructor and thus window is currently NULL
	if( window == NULL )
		window = new SAMPLE[N];
		
	double omega = 2.0 * MY_PI / (N - 1);
	SAMPLE normalizationFactor = 1.0 / (0.5 * N - 0.5);	//Normalize so window sums to one (unity gain at DC)
	
	for( int n = 0; n < N; n++)
		window[n] = (SAMPLE) normalizationFactor * ( 0.5 * (1.0 - cos(omega * n)) );
}


//Hamming: ~ -41dB sidelobes, -6dB / octave roll off, 4 omega_M wide main lobe
void WindowedFftHandler :: setWindowToHamming()
{
	//Allocate memory if this is being called from the constructor and thus window is currently NULL
	if( window == NULL )
		window = new SAMPLE[N];
		
	double omega = 2.0 * MY_PI / (N - 1);
	SAMPLE normalizationFactor = 	1.0 / (0.54 * N - 0.46); //Normalize so window sums to one (unity gain at DC)

	for( int n = 0; n < N; n++)
		window[n] = (SAMPLE) normalizationFactor * ( 0.54 - 0.46 * cos(omega * n));
}


//Classic Blackman: ~ -58dB sidelobes, -18dB / octave roll off, 6 omega_M wide main lobe
void WindowedFftHandler :: setWindowToBlackman()
{
	//Allocate memory if this is being called from the constructor and thus window is currently NULL
	if( window == NULL )
		window = new SAMPLE[N];
		
	double omega = 2.0 * MY_PI / (N - 1) ;
	SAMPLE normalizationFactor = 1.0 / ( 0.42 * N - 0.5 + 0.08); //Normalize so window sums to one (unity gain at DC)
	
	for( int n = 0; n < N; n++)
		window[n] = (SAMPLE) normalizationFactor * ( 0.42 - 0.5 * cos(omega * n) + 0.08 * cos(2 * omega * n));
}