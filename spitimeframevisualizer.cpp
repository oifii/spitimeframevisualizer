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
#include "RtAudio.h"
#include "ChannelBuffersHandler.h"
#include "SpectrumDisplay.h"
#include "AutocorrDisplay.h"
#include "WaveformDisplay.h"
#include "AutocorrHandler.h"
#include "defs.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>


#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#elif defined(__WINDOWS_DS__)
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif


using namespace std;


struct Point3D {
	
	GLfloat x;
	GLfloat y;
	GLfloat z;
	
};

//Window sizing variables
GLsizei screenWidth = 1280;
GLsizei screenHeight = 800;
double windowSizeFraction = 1.0;
GLsizei g_windowWidth = screenWidth * windowSizeFraction;
GLsizei g_windowHeight = screenWidth * windowSizeFraction;
GLboolean g_fullscreen = false;
//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
void idleFunc( );
void displayFunc( );
void reshapeFunc( GLsizei width, GLsizei height );
void keyboardFunc( unsigned char, int, int );
void keyboardSpecial( int key, int x, int y);
void mouseFunc( int button, int state, int x, int y );
void switchTo2D();
void switchTo3D();


//Global buffers -------------------------
ChannelBuffersHandler* callbackFilledAudioBuffers;
ChannelBuffersHandler* drawnFromAudioBuffers;


//Waterfall Objects ------------------------
SpectrumDisplay* specDisplayMix;
float** waterfallDisplayYValuesHistory;
int waterfallCount;
int waterfallReadIndex;
int waterfallWriteIndex;


//Other Display Objects ------------------------
AutocorrDisplay* autocorrDisplayL;
WaveformDisplay* waveformDisplayL;

AutocorrHandler* autocorrHandler;
SAMPLE* autocorrBuffer;
int trailCount;
int trailReadIndex;
int trailWriteIndex;					
int autocorrSkip1;
int autocorrSkip2;
GLfloat scaleFactor3D;
GLfloat scaleFactor2D;

Point3D** displayPointsHistory1;
Point3D** displayPointsHistory2;

float theta;
bool showWindows;
bool show3D;

//Global thread locking / management variables --------------
pthread_mutex_t g_mutex;
GLboolean g_ready = false;


//Audio callback fn ----------------------------------------------------------------
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double streamTime, RtAudioStreamStatus status, void* noData )
{
  //output warning if stream status isn't good
  if ( status ) cerr << "Stream over/underflow detected." << endl;
  
  //recast output and input buffer pointers
  SAMPLE *oSamples = (SAMPLE *) outputBuffer;
  SAMPLE *iSamples = (SAMPLE *) inputBuffer;  

  pthread_mutex_lock(&g_mutex);
	  
	  //Copy input buffer to callbackFilledAudioBuffers and output (pass through to speakers)
		callbackFilledAudioBuffers->acquireAndSeparateNewBuffer( iSamples );
		memcpy(oSamples, iSamples, nBufferFrames * 2 * sizeof(SAMPLE));	//Assumes 2 channels in / out
	  
	  g_ready = true;
	  
  pthread_mutex_unlock(&g_mutex);
   
  return 0;
}


//Main -----------------------------------------------------------------------------
int main( int argc, char ** argv )
{	
	// initialize GLUT
	glutInit( &argc, argv );
	// double buffer, use rgb color, enable depth buffer				//RGBA for alpha transparency
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	// initialize the window size
	glutInitWindowSize( g_windowWidth, g_windowHeight );
	// set the window postion
	glutInitWindowPosition( 0, 0 );
	// create the window
	glutCreateWindow( "Timeframe Visualizer" );
	
	//spi, begin
	HDC myDC = wglGetCurrentDC();
	HWND myHWND = WindowFromDC(myDC);
	SetWindowLong(myHWND, GWL_EXSTYLE, GetWindowLong(myHWND, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(myHWND, 0, 150, LWA_ALPHA);
	//spi, end

	// set the idle function - called when idle
	glutIdleFunc( idleFunc );
	// set the display function - called when redrawing
	glutDisplayFunc( displayFunc );
	// set the reshape function - called when client area changes
	glutReshapeFunc( reshapeFunc );
	// set the keyboard function - called on keyboard events
	glutKeyboardFunc( keyboardFunc );
	// set the special keyboard keys function - called on special keyboard events
	glutSpecialFunc( keyboardSpecial);
	// set the mouse function - called on mouse stuff
	glutMouseFunc( mouseFunc );
	
	glutSetCursor(GLUT_CURSOR_NONE);

	//spi, begin
	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));
	//spi, end

 //Bufferframs = 2048 -> fade block alpha 0.15
	// RtAudio Variables ------------------------------------------------------------
  //unsigned int inChannels = 2, outChannels = 2, fs = SRATE, bufferFrames = 1024, iDevice = 0, oDevice = 0, iOffset = 0, oOffset = 0; //original
  unsigned int inChannels = 2, outChannels = 2, fs = SRATE, bufferFrames = 1024, iDevice = 0, oDevice = 0, iOffset = 0, oOffset = 0; //spi
  
  // Initialize global buffers and objects
	callbackFilledAudioBuffers = new ChannelBuffersHandler( bufferFrames, inChannels );
	drawnFromAudioBuffers = new ChannelBuffersHandler( bufferFrames, inChannels );
	
	waveformDisplayL = new WaveformDisplay( bufferFrames, 10, 380, 10, 80, 0, bufferFrames, -1.0, 1.0);
	autocorrDisplayL = new AutocorrDisplay( bufferFrames, 10, 380, 10, 80, 0, bufferFrames, -8, 8);	
	specDisplayMix = new SpectrumDisplay( bufferFrames, 2*bufferFrames, "Hamming", "log", -7, 14, -5, 9, 0, SRATE/2, -50.0, 20.0 );
	
	//Initialize watefall variables
	//waterfallCount = 20; //original
	waterfallCount = 2; //spi
	waterfallDisplayYValuesHistory = new float*[waterfallCount];
	for( int i = 0; i < waterfallCount; i++)
	{
		waterfallDisplayYValuesHistory[i] = new float[2*bufferFrames];
	 	memset(waterfallDisplayYValuesHistory[i], -30, 2*bufferFrames * sizeof(float));	
	}
	
	waterfallReadIndex = 1;
	waterfallWriteIndex = 0;
	
	
	// Initialize the coolness variables
	autocorrHandler = new AutocorrHandler( bufferFrames );
	//	trailCount = 10;	//2048 buffersize
	//trailCount = 20; //original
	trailCount = 2; //spi
	autocorrBuffer = new SAMPLE[bufferFrames];
 	memset(autocorrBuffer, 0.0, bufferFrames * sizeof(SAMPLE));	
 

	displayPointsHistory1 = new Point3D*[trailCount];
	for(int i = 0; i < trailCount; i++)
	{
		displayPointsHistory1[i] = new Point3D[bufferFrames];
	 	memset(displayPointsHistory1[i], 0, bufferFrames * sizeof(Point3D));
	}
	
	
	displayPointsHistory2 = new Point3D*[trailCount];
	for(int i = 0; i < trailCount; i++)
	{
		displayPointsHistory2[i] = new Point3D[bufferFrames];
	 	memset(displayPointsHistory2[i], 0, bufferFrames * sizeof(Point3D));
	}

	trailReadIndex = 1;
	trailWriteIndex = 0;
	autocorrSkip1 = 10; //original
	//autocorrSkip1 = 1; //spi
	autocorrSkip2 = 80; //original
	//autocorrSkip2 = 2; //spi
	
	
	//Set display variables
	scaleFactor3D = 0.015*screenWidth/1280*4;	
	scaleFactor2D = 2.4*screenWidth/1280*4;
	theta = 0.0;
	showWindows = false;
	show3D = true;
	
	pthread_mutex_init(&g_mutex, NULL);

  RtAudio adac;
  if ( adac.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 1 );
  }

  // Let RtAudio print messages to stderr.
  adac.showWarnings( true );

  // Set the number of channels for input and output.
  RtAudio::StreamParameters iParams, oParams;
  
  // Input
  if ( iDevice == 0 )
    iParams.deviceId = adac.getDefaultInputDevice();
  else
    iParams.deviceId = iDevice - 1;
    
  iParams.nChannels = inChannels;
  iParams.firstChannel = iOffset;  
  
  // Output
  if ( oDevice == 0 )
    oParams.deviceId = adac.getDefaultOutputDevice();
  else
    oParams.deviceId = oDevice - 1;
    
  oParams.nChannels = outChannels;
  oParams.firstChannel = oOffset;

	//Create options struct in case we want to set some of these
  RtAudio::StreamOptions options;

  try {
  		adac.openStream( &oParams, &iParams, RTAUDIO_FLOAT64, fs, &bufferFrames, &tick, (void*) NULL, &options );
  }
  catch ( RtError& err ) {
    err.printMessage();
    exit( 1 );
  }

	// Try to open audio stream
  try {
    adac.startStream();
  }
  catch ( RtError& err ) {
    err.printMessage();
    goto cleanup;
  }
  
  
	//Enable alpha blending
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cout << "Keyboard Controls:" << endl;
	cout << "  Space Bar - Toggle 2D / 3D mode" << endl;
	cout << "  Tab - Toggle signal display windows in 3D mode" << endl;
	cout << "  l - Toggle lin/log freq scaling in waterfall (display window in 3D mode)" << endl;
	cout << "  Up / Down arrows - Zoom in / out" << endl;
  
  // Main loop -----------------------------------------
	// let GLUT handle the current thread from here
				glutMainLoop();
  // ---------------------------------------------------

  // If we get here, stop the stream.
  try
  {
    adac.stopStream();
  }
  catch( RtError& err )
  {
    err.printMessage();
  }


cleanup:
  if ( adac.isStreamOpen() ) adac.closeStream();

  delete callbackFilledAudioBuffers;
  delete drawnFromAudioBuffers;
	delete specDisplayMix;
	delete waveformDisplayL;
	delete autocorrDisplayL;
	delete autocorrHandler;
	
	delete []autocorrBuffer;

	for( int i=0; i < trailCount; i++ )
	{
		delete []displayPointsHistory1[i];
		delete []displayPointsHistory2[i];
	}
	
	delete []displayPointsHistory1;
	delete []displayPointsHistory2;	
	
	for( int i=0; i < waterfallCount; i++ )
		delete []waterfallDisplayYValuesHistory[i];
		
	delete []waterfallDisplayYValuesHistory;

  return 0;
}



//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc( GLsizei w, GLsizei h )
{
    // save the new window size
    g_windowWidth = w; g_windowHeight = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    
	if( show3D )
	{    
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 1.0, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
	}
	else
	{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    glutSwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    glutSwapBuffers();    
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // set to ortho for 2D version
		gluOrtho2D(-g_windowWidth/2, g_windowWidth/2, -g_windowHeight/2, g_windowHeight/2);
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );

	}
}


//Function to deal with special keys (arrow keys)
void keyboardSpecial( int key, int x, int y)
{
	switch( key )
	{
		//Rotate left a small chunk
		case GLUT_KEY_LEFT:
			if( show3D )
		    theta -= MY_PI / 50;
	    break;
		
		//Rotate right a small chunk
	  case GLUT_KEY_RIGHT:
	  	if( show3D )
		  	theta += MY_PI / 50;
	  	break;
	  
	  //Zoom in	
	  case GLUT_KEY_UP:
	  	if( show3D )
		  	scaleFactor3D *= 1.05;
	  	else
	  		scaleFactor2D *= 1.05;
	  		
	  	break;
	  
	  //Zoom out
  	case GLUT_KEY_DOWN:
  		if( show3D )
	  		scaleFactor3D *= 0.95;
  		else
  			scaleFactor2D *= 0.95;
  		break;
	}
    

}


//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
        case 'Q':
        case 'q':
 					exit(1);
 					break;
 					
        case 'z':
	        if( !g_fullscreen )
						glutFullScreen();
	        else
						glutReshapeWindow( g_windowWidth, g_windowHeight );
		        g_fullscreen = !g_fullscreen;
        	break;
        	
        case 9:	//tab key toggles little window plots in 3D mode
        	if( show3D )
        	{
        		switchTo3D();	//Reload 3D perspective and viewport, since we only do this in the loop if we are continuously drawing windows
	        	showWindows = !showWindows;
        	}
        	break;
        	
       	case 32: //space key toggles 3D / 2D
       		if( show3D )
       			switchTo2D();
     			else
     				switchTo3D();
     				
       		show3D = !show3D;
       		break;
        	
        case 'l':	//l key toggles log (really ^0.3) / lin axis in waterfall plot
        	char* displayLogLin = new char[20];
        	specDisplayMix->copyOutAxisType(displayLogLin);
        	
        	if ( strcmp(displayLogLin, "Linear") == 0 )
        		specDisplayMix->setAxisType("Log");
        	else
        		specDisplayMix->setAxisType("Linear");	       		

       		break;
        
    }
    
    glutPostRedisplay( );
}


//This function switches from 3D to 2D mode, adjusting viewport parameters and such.
void switchTo2D()
{
    
		// set the viewPort to the entire screen
    glViewport( 0, 0, g_windowWidth, g_windowHeight );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // set to ortho for 2D version
		gluOrtho2D(-g_windowWidth/2, g_windowWidth/2, -g_windowHeight/2, g_windowHeight/2);
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
    
    //Enable line smoothing
		glEnable(GL_LINE_SMOOTH);
		glBlendFunc(GL_LINE_SMOOTH_HINT, GL_NICEST);
		
		//Clear color buffer for transition since 2D doesn't clear on updates
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    glutSwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    glutSwapBuffers();    
}


//This function switches from 2D to 3D mode, adjusting viewport parameters and such.
void switchTo3D()
{		
    // map the view port to the client area
    glViewport( 0, 0, g_windowWidth, g_windowHeight );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) g_windowWidth / (GLfloat) g_windowHeight, 1.0, 300.0 );
    
    //Disable line smoothing
		glDisable(GL_LINE_SMOOTH);
}


//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // when left mouse button is down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        // when right mouse button down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else
    {
    }
    
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
 // ---------------- Wait and Grab New Audio  ---------------------- 
	
	//Don't do anything until we have new audio data    
  //while( !g_ready ) usleep( 1000 );
  while( !g_ready ) Sleep( 10 ); //spi
  //while( !g_ready ) Sleep( 1 ); //spi
		
 	//Lock while copying so the contents of the callbackFilledAudioBuffers don't change during copy
  pthread_mutex_lock(&g_mutex);
     
  	//Copy the latest audio buffers filled by the audio callback function
  	drawnFromAudioBuffers->copyInBuffersFromHandler( callbackFilledAudioBuffers );
  	//mark that we don't have new audio data yet
	  g_ready = false;
	    
  pthread_mutex_unlock(&g_mutex);


 // -------------------- Draw 3D  -------------------------- 	
if( show3D )
{
	
 // -------------------- Draw the Coolness  -------------------------- 	

		//Reset viewport and projection if we're drawing windows
		if( showWindows )
		{
	    // map the view port to the client area
	    glViewport( 0, 0, g_windowWidth, g_windowHeight );
	    // set the matrix mode to project
	    glMatrixMode( GL_PROJECTION );
	    // load the identity matrix
	    glLoadIdentity( );
	    // create the viewing frustum
	    gluPerspective( 45.0, (GLfloat) g_windowWidth / (GLfloat) g_windowHeight, 1.0, 300.0 );
 		}

    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point - view rotates around
    gluLookAt( 5.0*sin(theta), 5.0*sin(theta), 10.0*cos(theta), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
		
		theta += MY_PI / (6*43.06); //original
		//theta += rand()*MY_PI / (6*43.06); //spi
		//theta += 0; //spi
    
    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    
    glPushMatrix();

	    //copy out Left channel buffer
		 	drawnFromAudioBuffers->copyOutChannelBuffer(autocorrBuffer, 0);
	 	 
	 	 	//update read index and and calculate autocorrelation
 	 	  trailReadIndex = trailWriteIndex;
 	 	  autocorrHandler->autocorr( autocorrBuffer, autocorrBuffer );
 	 	  
			//Calculate the plot point locations for the lines to be drawn, given the just calculated autocorrelation function
			//Spacing 1 (white color)
    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip1*5-4; i=i+2 )
    	{
    		displayPointsHistory1[trailWriteIndex][i].x = scaleFactor3D * autocorrBuffer[i];
    		displayPointsHistory1[trailWriteIndex][i].y = -scaleFactor3D * autocorrBuffer[i+autocorrSkip1];
    		displayPointsHistory1[trailWriteIndex][i].z = scaleFactor3D * autocorrBuffer[i+2*autocorrSkip1];
    		
    		displayPointsHistory1[trailWriteIndex][i+1].x = scaleFactor3D * autocorrBuffer[i+3*autocorrSkip1];
    		displayPointsHistory1[trailWriteIndex][i+1].y = -scaleFactor3D * autocorrBuffer[i+4*autocorrSkip1];
    		displayPointsHistory1[trailWriteIndex][i+1].z = scaleFactor3D * autocorrBuffer[i+5*autocorrSkip1];
    	}
    	
    	//Spacing 2 (blue color)
    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip2*5-4; i=i+2 )
    	{
    		displayPointsHistory2[trailWriteIndex][i].x = scaleFactor3D * autocorrBuffer[i];
    		displayPointsHistory2[trailWriteIndex][i].y = -scaleFactor3D * autocorrBuffer[i+autocorrSkip2];
    		displayPointsHistory2[trailWriteIndex][i].z = scaleFactor3D * autocorrBuffer[i+2*autocorrSkip2];
    		
    		displayPointsHistory2[trailWriteIndex][i+1].x = scaleFactor3D * autocorrBuffer[i+3*autocorrSkip2];
    		displayPointsHistory2[trailWriteIndex][i+1].y = -scaleFactor3D * autocorrBuffer[i+4*autocorrSkip2];
    		displayPointsHistory2[trailWriteIndex][i+1].z = scaleFactor3D * autocorrBuffer[i+5*autocorrSkip2];
    	}
			
			//Decrement write pointer (displayPointsHistory keeps a circular buffer)
			trailWriteIndex--;			
 	 	  if( trailWriteIndex < 0 )
	 	  	trailWriteIndex = trailCount - 1;	  	
	 	
 		 	//Set line width
	    //glLineWidth( 1.1 );	//original 	
	    //glLineWidth( 5.5 );	//spi 	
	    //glLineWidth( 0.5 );	//spi 	
	    //glLineWidth( 20.0 );	//spi 	
	    //glLineWidth( 100.0 );	//spi 	
	 	//glPointSize(50.0f); //spi
	 	glPointSize(10.0f); //spi

    	glPushMatrix();
    	
	    	int drawIndex;
				
				//draw from oldest to newest (actually just need to make sure we set the alpha to the lowest on the oldest plot)
	    	for( int plotNum = trailCount - 1; plotNum > -1; plotNum-- )
	    	{
	    		if(	trailReadIndex + plotNum >= trailCount )
	    			drawIndex = trailReadIndex + plotNum - trailCount;
	   			else
	   				drawIndex = trailReadIndex + plotNum;
	
					//Set color to blue, alpha geometrically decreasing for older plots
//	   			glColor4f(0.0 , 136/255.0, 158/255.0, pow(0.8, plotNum)*0.7); //2048 buffersize
//	   			glColor4f(0.0 , 136/255.0, 158/255.0, pow(0.89, plotNum)*0.7); //1024 buffersize //original
	   			glColor4f(136/255.0 , 0.0, 158/255.0, pow(0.89, plotNum)*0.7); //1024 buffersize //spi
	   			
	 				//Plot the blue timeSkip2 lines - these points were ordered previously when calculating points from the autocorrelation function
//		    	glBegin( GL_LINES ); //original
				glBegin(GL_POINTS); //spi
			    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip2*5-4; i++ )
			    	{
		    			glVertex3f(displayPointsHistory2[drawIndex][i].x, displayPointsHistory2[drawIndex][i].y, displayPointsHistory2[drawIndex][i].z);
			    	}
		    	glEnd();		   				
	
					//Set color to white, alpha geometrically decreasing for older plots
//	   			glColor4f(1.0, 1.0, 1.0, pow(0.8, plotNum)*0.8);	//2048 buffersize
//	   			glColor4f(1.0, 1.0, 1.0, pow(0.88, plotNum)*0.8); //original
	   			glColor4f(0.0, 0.0, 1.0, pow(0.88, plotNum)*0.8); //spi
	
	 				//Plot the white timeSkip1 lines - these points were ordered previously when calculating points from the autocorrelation function   			
//		    	glBegin( GL_LINES ); //original
				glBegin(GL_POINTS); //spi
			    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip1*5-4; i++ )
			    	{
		    			glVertex3f(displayPointsHistory1[drawIndex][i].x, displayPointsHistory1[drawIndex][i].y, displayPointsHistory1[drawIndex][i].z);
			    	}
		    	glEnd();   	
	    	}
	    	
    	glPopMatrix();

		glPopMatrix();
		

// ---------------------- Coolness end ---------------------------	 	  
	 	  
	 	  
// -------------------- Draw windows --------------------------
	if( showWindows )
	{
		
		// ---------------- Autocorrelation window ----------------
		glPushMatrix();
		
	    // map the view port to the autocorr window location
	    glViewport( 20, 10, 400, 100 );
	    // set the matrix mode to project
	    glMatrixMode( GL_PROJECTION );
	    // load the identity matrix
	    glLoadIdentity( );
	    // working in ortho mode for this line plot
			gluOrtho2D(0, 400, 0, 100);
	    // set the matrix mode to modelview
	    glMatrixMode( GL_MODELVIEW );
	    // load the identity matrix
	    glLoadIdentity( );
 	    // position the view point
		  gluLookAt( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
		  
		  // Draw black mostly opaque background
	    glColor4f( 0.0, 0.0, 0.0, 0.85);

	    glBegin( GL_QUADS );
	    	glVertex3f( 0.0, 0.0, 0.0 );
	    	glVertex3f( 0.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 0.0, 0.0 );
	    glEnd();
		  
			//Grab left channel audio from the buffer and generate the display from this buffer
			//  (calculates autocorrelation and scales the output to fit the given space, with specified clipping limits)
 			drawnFromAudioBuffers->copyOutChannelBuffer( autocorrDisplayL->realValuedInputBuffer, 0);
			autocorrDisplayL->generateDisplayFromBuffer();
			
			//Get pointers to the display coordinate values found above
			const float* xVals = autocorrDisplayL->getDisplayXValuesPointer();
			const float* yVals = autocorrDisplayL->getDisplayYValuesPointer();
		
 			glColor4f(1.0, 1.0, 1.0, 1.0);
 			
 			//Plot the line
	    glPushMatrix();
	    	glBegin( GL_LINE_STRIP );
	    	for( GLint i = 0; i < autocorrDisplayL->getPlotLength(); i++ )
    			glVertex2f(*xVals++, *yVals++);
	    	glEnd();
    	glPopMatrix();
 
 	  glPopMatrix();
 	  
		 // ----------------- Autocorr window  end --------------------	
 
		 // ------------------ Waveform window ------------------------

		glPushMatrix();
		
	    // map the view port to the client area
	    glViewport( 440, 10, 400, 100 );
	    // set the matrix mode to project
	    glMatrixMode( GL_PROJECTION );
	    // load the identity matrix
	    glLoadIdentity( );
	    // working in ortho mode for this line plot
			gluOrtho2D(0, 400, 0, 100);
	    // set the matrix mode to modelview
	    glMatrixMode( GL_MODELVIEW );
	    // load the identity matrix
	    glLoadIdentity( );
 	    // position the view point
		  gluLookAt( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );

		  // Draw black mostly opaque background
	    glColor4f( 0.0, 0.0, 0.0, 0.85);
	    glBegin( GL_QUADS );
	    	glVertex3f( 0.0, 0.0, 0.0 );
	    	glVertex3f( 0.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 0.0, 0.0 );
	    glEnd();


			//Grab left channel audio from the buffer and generate the display from this buffer
			//  (scales the waveform to fit the given space, with specified clipping limits)
			drawnFromAudioBuffers->copyOutChannelBuffer( waveformDisplayL->realValuedInputBuffer, 0);
			waveformDisplayL->generateDisplayFromBuffer();
			
			//Get pointers to the display coordinate values found above			
			xVals = waveformDisplayL->getDisplayXValuesPointer();
			yVals = waveformDisplayL->getDisplayYValuesPointer();			
			
			
	    glColor3f( 1.0, 1.0, 1.0);	    
	    
	    //Plot the line
	    glPushMatrix();
	    	glBegin( GL_LINE_STRIP );
				for( GLint i = 0; i < waveformDisplayL->getPlotLength(); i++)
    			glVertex2f(*xVals++, *yVals++);
	    	glEnd();
    	glPopMatrix();

		glPopMatrix();	  
 
		// --------------- Spectrum window end ---------------------	
 
		// ---------------- Waterfall window -----------------------	

		glPushMatrix();

	    // map the view port to the client area
	    glViewport( 860, 10, 400, 100 );
	    // set the matrix mode to project
	    glMatrixMode( GL_PROJECTION );
	    // load the identity matrix
	    glLoadIdentity( );
			// working in ortho for this line plot
			gluOrtho2D(0, 400, 0, 100);
	    // set the matrix mode to modelview
	    glMatrixMode( GL_MODELVIEW );
	    // load the identity matrix
	    glLoadIdentity( );
 	    // position the view point
		  gluLookAt( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );		

		  // Draw black mostly opaque background
	    glColor4f( 0.0, 0.0, 0.0, 0.85);
	    glBegin( GL_QUADS );
	    	glVertex3f( 0.0, 0.0, 0.0 );
	    	glVertex3f( 0.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 100.0, 0.0 );
	    	glVertex3f( 400.0, 0.0, 0.0 );
	    glEnd();
		    

			glPushMatrix();
				
				glViewport( 870, 13, 380, 88 );
				//Switch back to perspective mode, within this window, in order to plot the waterfall with 3D perspective
		    glMatrixMode( GL_PROJECTION );
		    // load the identity matrix
		    glLoadIdentity( );
		    // create the viewing frustum
		    gluPerspective( 45.0, (GLfloat) g_windowWidth / (GLfloat) g_windowHeight, 1.0, 300.0 );
		    // set the matrix mode to modelview
		    glMatrixMode( GL_MODELVIEW );
		    // load the identity matrix
		    glLoadIdentity( );
	 	    // position the view point
			  gluLookAt( 0.0f, 3.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
	
				
				//Update the read index and store the last display  values in the circular history buffer
		 	  waterfallReadIndex = waterfallWriteIndex;
		 	  specDisplayMix->copyDisplayYValues( waterfallDisplayYValuesHistory[waterfallWriteIndex--] );
		 	  
		 	  if( waterfallWriteIndex < 0 )
		 	  	waterfallWriteIndex = waterfallCount - 1;
				
				//Get a mix of the channels and generate the spectral display from it with the specified parameters
				SAMPLE mixCoeffs[2] = {0.707, 0.707};
				drawnFromAudioBuffers->copyOutMixOfChannelBuffers( specDisplayMix->realValuedInputBuffer, mixCoeffs );
				specDisplayMix->generateDisplayFromBuffer();
				
				//Get pointers to the display coordinate values found above	
				xVals = specDisplayMix->getDisplayXValuesPointer();
				yVals = specDisplayMix->getDisplayYValuesPointer();			
		    
    		//Plot current spectrum
	    	glBegin( GL_LINE_STRIP );
	    	for( GLint i = 0; i < specDisplayMix->getPlotLength(); i++ )
	    	{
    			glVertex3f(xVals[i], yVals[i], 0.0);
	    	}
	    	glEnd();

	
	    	//Plot waterfall - plot from oldest to newest so we can get the darkness of the lines right
	    	int drawIndex;
	    	for( int plotNum = waterfallCount - 1; plotNum >= 0; plotNum-- )
	    	{
	    		if(	waterfallReadIndex + plotNum >= waterfallCount )
	    			drawIndex = waterfallReadIndex + plotNum - waterfallCount;
	   			else
	   				drawIndex = waterfallReadIndex + plotNum;
	  
	  			//Older plots are darker
	   			glColor3f(1.0 - 2.0*plotNum*(1.0/waterfallCount), 1.0 -  2.0*plotNum*(1.0/waterfallCount), 1.0 -  2.0*plotNum*(1.0/waterfallCount));
	   			
		    	glBegin( GL_LINE_STRIP );
			    	for( GLint i = 0; i < specDisplayMix->getPlotLength(); i++ )
			    	{
		    			glVertex3f(xVals[i], waterfallDisplayYValuesHistory[drawIndex][i], (plotNum + 1) * -1.0);
			    	}
		    	glEnd();
	    	}
	    	
   		glPopMatrix();
		
		glPopMatrix();  
		
		// ---------------- Waterfall window end ----------------------						
	}	  
	
// ------------------ Draw windows end ------------------------

  // flush!
  glFlush( );
  // swap the double buffer
  glutSwapBuffers( );

}
// ------------------ Draw 3D end ------------------------				  

// -------------------- Draw 2D --------------------------
else
{
	//We're doing alpha fading by drawing an mostly transparent black rectangle over the whole scene between frames.
	//	This really should use single buffering, but since the program should be double buffered to support the 3D
	//	version, we just draw the same thing twice here to put it on both buffers. This seems to work okay.
	for( int buffer = 0; buffer < 2; buffer++ )
	{
		
		//alpha fade what's there
//		2048 buffer -> 0.15
    glColor4f( 0.0, 0.0, 0.0, 0.04);
    glBegin( GL_QUADS );
    	glVertex3f( -g_windowWidth/2, -g_windowHeight/2, 0.0 );
    	glVertex3f( -g_windowWidth/2, g_windowHeight/2, 0.0 );
    	glVertex3f( g_windowWidth/2, g_windowHeight/2, 0.0 );
    	glVertex3f( g_windowWidth/2, -g_windowHeight/2, 0.0 );
    glEnd();
		
	// ------------------- Draw 2D Coolness -------------------------	 	  		
    
    //copy out Left channel buffer and calculate autocorrelation
   	drawnFromAudioBuffers->copyOutChannelBuffer(autocorrBuffer, 0);
 	  autocorrHandler->autocorr( autocorrBuffer, autocorrBuffer );

		glPushMatrix();    	
		
			//Plot blue lines
	    glLineWidth( 1.0 );			
 			glColor4f(0.0 , 136/255.0, 158/255.0, 0.4);
  			   			
    	glBegin( GL_LINES );
	    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip2*3-4; i=i+2 )
	    	{
    			glVertex3f(scaleFactor2D * autocorrBuffer[i], -scaleFactor2D * autocorrBuffer[i+autocorrSkip2], 0.0);
    			glVertex3f(scaleFactor2D * autocorrBuffer[i+2*autocorrSkip2], -scaleFactor2D * autocorrBuffer[i+3*autocorrSkip2], 0.0);	    		
    		}
    	glEnd();
    	
   	glPopMatrix();  				
   	

		glPushMatrix();    	
		
			//Plot white lines
	    glLineWidth( 1.0 );			
 			glColor4f(1.0, 1.0, 1.0, 1.0);		    
  			   			
    	glBegin( GL_LINES );
	    	for( GLint i = 0; i < autocorrHandler->getN() - autocorrSkip2*3-4; i=i+2 )
	    	{
    			glVertex3f(scaleFactor2D * autocorrBuffer[i], -scaleFactor2D * autocorrBuffer[i+autocorrSkip1], 0.0);
    			glVertex3f(scaleFactor2D * autocorrBuffer[i+2*autocorrSkip1], -scaleFactor2D * autocorrBuffer[i+3*autocorrSkip1], 0.0);	    		
    		}
    	glEnd();
    	
   	glPopMatrix();  				
	  
	// -------------------- 2D Coolness end --------------------------	 	  	  
	
  // flush!
  glFlush( );
  glutSwapBuffers();
	}

}
    
}
