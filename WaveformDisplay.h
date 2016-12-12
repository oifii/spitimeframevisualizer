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