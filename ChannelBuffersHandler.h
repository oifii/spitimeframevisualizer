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