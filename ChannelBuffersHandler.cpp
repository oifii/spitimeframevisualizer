#include "ChannelBuffersHandler.h"

ChannelBuffersHandler :: ChannelBuffersHandler(long singleChanBufferLengthIn, int numberOfChannelsIn)
{
	singleChanBufferLength = singleChanBufferLengthIn;
	numberOfChannels = numberOfChannelsIn;
	
	//Initialize buffers array
	singleChanBuffers = new SAMPLE*[numberOfChannels];
	for(int i = 0; i < numberOfChannels; i++)
		singleChanBuffers[i] = new SAMPLE[singleChanBufferLength];
		
}


ChannelBuffersHandler :: ~ChannelBuffersHandler()
{
	for(int i = 0; i < numberOfChannels; i++)
		delete []singleChanBuffers[i];
		
	delete []singleChanBuffers;	
}

//splits the interleaved inBuffer into individual channels and stores a local copy of them
void ChannelBuffersHandler :: acquireAndSeparateNewBuffer(SAMPLE* inBuffer)
{
	int sampleNum = 0;
	
	for(int i = 0; i < singleChanBufferLength; i++)
		for(int j = 0; j < numberOfChannels; j++)
			singleChanBuffers[j][i] = inBuffer[sampleNum++];
}

//copies channel buffers from another ChannelBuffersHandler
void ChannelBuffersHandler :: copyInBuffersFromHandler(ChannelBuffersHandler* inBuffersHandler)
{
	//Make sure this handler has the same length buffers and number of channels as the one to copy from
	assert(inBuffersHandler->getSingleChanBufferLength() == singleChanBufferLength);
	assert(inBuffersHandler->getNumberOfChannels() == numberOfChannels);
	
	//copy each of the other handler's buffers into this handler's single channel buffers, in order
	for( int currentChannel = 0; currentChannel < numberOfChannels; currentChannel++ )
		inBuffersHandler->copyOutChannelBuffer( singleChanBuffers[currentChannel], currentChannel );
}

//copies the specified channel buffer into the given destination	
void ChannelBuffersHandler :: copyOutChannelBuffer(SAMPLE* copyDestination, int channelNum)
{
	memcpy(copyDestination, singleChanBuffers[channelNum], singleChanBufferLength * sizeof(SAMPLE));
}


//copies a mix of the channel buffer contents into the copyDestination array specified by the coefficients in the channelMixCoeffs array
void ChannelBuffersHandler :: copyOutMixOfChannelBuffers(SAMPLE* copyDestination, SAMPLE* channelMixCoeffs)
{
	//Skip through to the first non-zero weight. We assume weights are positive.
	int firstChannel = 0;
	while( channelMixCoeffs[firstChannel] <= 0.0000000001 )
		firstChannel++;
		
	//initialize with first non-zero weighted mix component
	for(int i = 0; i < singleChanBufferLength; i++)
		copyDestination[i] = channelMixCoeffs[firstChannel] * singleChanBuffers[firstChannel][i];
	
	//sum in the rest of the non-zero weighted mix components
	for(int currentChannel = firstChannel + 1; currentChannel < numberOfChannels; currentChannel++)
	{
		if( ! channelMixCoeffs[currentChannel] <= 0.0000000001 )
		{
			for(int i = 0; i < singleChanBufferLength; i++)
				copyDestination[i] += channelMixCoeffs[currentChannel] * singleChanBuffers[currentChannel][i];
		}	
	}
}