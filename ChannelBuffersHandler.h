#ifndef CHANNEL_BUFFERS_HANDLER
#define CHANNEL_BUFFERS_HANDLER

#include "defs.h"
#include <memory.h>
#include <cassert>


//Class which handles storage buffers for incoming audio

class ChannelBuffersHandler
{
public:
	
	ChannelBuffersHandler(long singleChanBufferLengthIn, int numberOfChannelsIn);	//Constructor
	
	~ChannelBuffersHandler();	//Destructor

	//splits the inBuffer into individual channels	
	void acquireAndSeparateNewBuffer(SAMPLE* inBuffer);
	
	//copies channel buffers from another ChannelBuffersHandler
	void copyInBuffersFromHandler(ChannelBuffersHandler* inBuffersHandler);

	//copies the specified channel buffer into the given destination	
	void copyOutChannelBuffer(SAMPLE* copyDestination, int channelNum);
	
	//copies a mix of the channel buffer contents into the copyDestination array specified by the coefficients in the channelMixCoeffs array
	void copyOutMixOfChannelBuffers(SAMPLE* copyDestination, SAMPLE* channelMixCoeffs);
	
	//returns a pointer to the specified channel buffer, using const to prevent external functions from changing the buffer contents. 
	//  can be used to read from channel buffer in place.
  const SAMPLE* getChannelBufferPointer( int channelNum ) { return (const SAMPLE*) singleChanBuffers[channelNum]; };
	
	int getNumberOfChannels() { return numberOfChannels; };
	long getSingleChanBufferLength() { return singleChanBufferLength; };
	
private:
	
	SAMPLE** singleChanBuffers;
	
	long singleChanBufferLength;
	int  numberOfChannels;
	
};
#endif