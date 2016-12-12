/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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