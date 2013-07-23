

// Include Headers
#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>

// Use the cv and std namespace (less typing)
using namespace cv;
using namespace std;

const char* source_window = "TEST"; // Name of the GUI Window

// Globals
int g_slider_position 	= 0;
CvCapture* g_capture	= NULL; // CvCapture structure object

/* Trackbar callback routine */
void onTrackbarSlide(int pos) {
	// Allow the user to configure various properties of CvCapture object
	cvSetCaptureProperty(
		g_capture, // CvCapture object
		CV_CAP_PROP_POS_FRAMES, // Argument which inidicate the read position in units of frames
		pos
	);
}

/* Main Program */
int main( int argc, char** argv ) {
	
	// Create a window ( TEST , autosize the window )
	cvNamedWindow( source_window, CV_WINDOW_AUTOSIZE );
	// Load the AVI video from a file into the CvCapture object
	g_capture = cvCreateFileCapture( "/home/trafficvision/Desktop/cars.avi" );
	
	/* FOR FUTURE USE 
	 *  Load the AVI videom from the CAMERA to the CvCapture structure
	 *  g_capture = cvCreateCameraCapture( -1 ); // -1 means pick one camera source
	 */
	
	
	// Use the GetCaptureProperty to query the CvCapture object for data (# of frames), store into frames
	int frames = (int) cvGetCaptureProperty(
		g_capture,
		CV_CAP_PROP_FRAME_COUNT // Find out how many frames are in AVI
	);
	// Create the trackbar
	if( frames != 0 ) {
		cvCreateTrackbar( 
			"Position", // Trackbar label
			source_window , // Specify window for trackbar
			&g_slider_position, // Adjust the value of g_slider_position
			frames, // Grab the # of frames in video
			onTrackbarSlide // Make a sliding trackbar
		);
	}
	// Load frame of video into structure IplImage
	IplImage* frame;
	
	// Main Loop
	while(1) {
		// Load the beginning of AVI file, take argument for CvCapture object
		// and store into frame memory (memory is part of CvCapture structure)
		frame = cvQueryFrame( g_capture );
		// If the frame is null, exit loop
		if( !frame ) break;
		// Display the frame into the window
		cvShowImage( source_window, frame );
		// If the user presses ESC, exit the program.
		char c = cvWaitKey(33);
		if( c == 27 ) break;
	}
	// Release memory allocated objects & kill window
	cvReleaseCapture( &g_capture );
	cvDestroyWindow( source_window );
}
