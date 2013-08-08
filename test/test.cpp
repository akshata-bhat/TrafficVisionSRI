/*
 * Traffic Vision 
 * 
 * This program detects moving vehicles, and displays it to a user.
 * 
 * Juan Carlos Vazquez (juan.vazquez006@mymdc.net)
 * Miami Dade College
 * Computing Research Lab
 * 
 * Coding credit to:
 * David Stavens (david.stavens@ai.stanford.edu)
 *
 */

// Include Headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

// Use the cv and std namespace (less typing)
using namespace cv;
using namespace std;

const char* source_window = "TEST"; // Name of the GUI Window

/// Globals
RNG rng(12345);
int g_slider_position 	= 0;
CvCapture* g_capture	= NULL; // CvCapture structure object
static const double pi = 3.14159265358979323846;

inline static double square(int a)
{
	return a * a;
}
/* This is just an inline that allocates images.  I did this to reduce clutter in the
 * actual computer vision algorithmic code.  Basically it allocates the requested image
 * unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 * if that image's size, depth, and/or channels are different than the request.
 */
inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}

/// Trackbar callback routine
void onTrackbarSlide(int pos) {
	// Allow the user to configure various properties of CvCapture object
	cvSetCaptureProperty(
		g_capture, // CvCapture object
		CV_CAP_PROP_POS_FRAMES, // Argument which inidicate the read position in units of frames
		pos
	); 
}

/// Main Program 
int main( int argc, char** argv ) {
	
	// Create a window ( TEST , autosize the window )
	cvNamedWindow( source_window, CV_WINDOW_AUTOSIZE );
	// Load the AVI video from a file into the CvCapture object
	g_capture = cvCreateFileCapture( "/home/trafficvision/Desktop/object.avi" );
	
	/* FOR FUTURE USE 
	 *  Load the AVI videom from the CAMERA to the CvCapture structure
	 *  g_capture = cvCreateCameraCapture( -1 ); // -1 means pick one camera source
	 */
	
	// If video does not load, display error message.
	if (g_capture == NULL)
	{
		fprintf(stderr, "Error: Can't open video.\n");
		return -1;
	}
	
	// Read the video's frame size out of the AVI.
	CvSize frame_size;
	frame_size.height = (int) cvGetCaptureProperty( g_capture, CV_CAP_PROP_FRAME_HEIGHT );
	frame_size.width  = (int) cvGetCaptureProperty( g_capture, CV_CAP_PROP_FRAME_WIDTH );
	
	// Use the GetCaptureProperty to query the CvCapture object for data (# of frames).
	// Store into frames.
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
	//IplImage* frame;
	
	/// Main Loop
	while(1) {
		// Initialize variables
		static IplImage *frame = NULL, 
					    *frame1 = NULL, 
						*frame1_1C = NULL, 
						*frame2_1C = NULL,
						*eigImage = NULL, 
						*tempImage = NULL,
						*pyramid1 = NULL,
						*pyramid2 = NULL;
		
	    // Get the first(next) frame of the video.
		// IMPORTANT!  cvQueryFrame() always returns a pointer to the _same_
		// memory location.  So successive calls:
		// frame1 = cvQueryFrame();
		// frame2 = cvQueryFrame();
		// frame3 = cvQueryFrame();
		// will result in (frame1 == frame2 && frame2 == frame3) being true.
		// The solution is to make a copy of the cvQueryFrame() output.
		frame = cvQueryFrame( g_capture );
		if (frame == NULL){
			fprintf(stderr, "Error: Hmm. The end came sooner than we thought.\n");
			return -1;
		}
		// Allocate another image if not already allocated.
		// Image has ONE channel of color (ie: monochrome) with 8-bit "color" depth.
		// This is the image format OpenCV algorithms actually operate on (mostly).
		allocateOnDemand( &frame1_1C, frame_size, IPL_DEPTH_8U, 1 );
		/* Convert whatever the AVI image format is into OpenCV's preferred format.
		 * AND flip the image vertically.  Flip is a shameless hack.  OpenCV reads
		 * in AVIs upside-down by default.  (No comment :-))
		 */
		//cvConvertImage(frame, frame1_1C, CV_CVTIMG_FLIP);
		// We'll make a full color backup of this frame so that we can draw on it.
		// (It's not the best idea to draw on the static memory space of cvQueryFrame().)
		allocateOnDemand( &frame1, frame_size, IPL_DEPTH_8U, 3 );
		//cvConvertImage(frame, frame1, CV_CVTIMG_FLIP);
		// Get the second frame of video.  Same principles as the first.
		frame = cvQueryFrame( g_capture );
		if (frame == NULL)
		{
			fprintf(stderr, "Error: Hmm. The end came sooner than we thought.\n");
			return -1;
		}
		allocateOnDemand( &frame2_1C, frame_size, IPL_DEPTH_8U, 1 );
		//cvConvertImage(frame, frame2_1C, CV_CVTIMG_FLIP);
		
		
		/// Implement Shi/Tomasi Good Feature Algorithm
		// Preparation: Allocate the necessary storage. 
		allocateOnDemand( &eigImage, frame_size, IPL_DEPTH_32F, 1 );
		allocateOnDemand( &tempImage, frame_size, IPL_DEPTH_32F, 1 );
		// Preparation: This array will contain the features found in frame 1. 
		CvPoint2D32f frame1_features[100];
		// Preparation: BEFORE the function call this variable is the array size
		// (or the maximum number of features to find).  AFTER the function call
		// this variable is the number of features actually found.
		int number_of_features;
		// I'm hardcoding this at 400.  But you should make this a #define so that you can
		// change the number of features you use for an accuracy/speed tradeoff analysis.
		number_of_features = 100;
		/*---Actually run the Shi and Tomasi algorithm!!---*/
		/* "frame1_1C" is the input image.
		 * "eigImage" and "tempImage" are just workspace for the algorithm.
		 * The first ".01" specifies the minimum quality of the features (based on the eigenvalues).
		 * The second ".01" specifies the minimum Euclidean distance between features.
		 * "NULL" means use the entire input image.  You could point to a part of the image.
		 * WHEN THE ALGORITHM RETURNS:
		 * "frame1_features" will contain the feature points.
		 * "number_of_features" will be set to a value <= 100 indicating the number of feature points found.
		 */ 
		cvGoodFeaturesToTrack(
			frame1_1C, 
			eigImage,
			tempImage,
			frame1_features,
			&number_of_features,
			.01, //quality level
			.01, //min distance
			NULL);
		
		/// Pyramidal Lucas Kanade Optical Flow!
		// This array will contain the locations of the points from frame 1 in frame 2. 
		CvPoint2D32f frame2_features[100];
		// The i-th element of this array will be non-zero if and only if the i-th feature of
		// frame 1 was found in frame 2.
		char optical_flow_found_feature[100];
		// The i-th element of this array is the error in the optical flow for the i-th feature
		// of frame1 as found in frame 2.  If the i-th feature was not found (see the array above)
		// I think the i-th entry in this array is undefined.
		float optical_flow_feature_error[100];
		// This is the window size to use to avoid the aperture problem (see slide "Optical Flow: Overview").
		CvSize optical_flow_window = cvSize(3,3);
		// This termination criteria tells the algorithm to stop when it has either done 20 iterations or when
		// epsilon is better than .3.  You can play with these parameters for speed vs. accuracy but these values
		// work pretty well in many situations.
		CvTermCriteria optical_flow_termination_criteria
			= cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );
		// This is some workspace for the algorithm.
		// (The algorithm actually carves the image into pyramids of different resolutions.)
		allocateOnDemand( &pyramid1, frame_size, IPL_DEPTH_8U, 1 );
		allocateOnDemand( &pyramid2, frame_size, IPL_DEPTH_8U, 1 );
		/*---Actually run Pyramidal Lucas Kanade Optical Flow!!---*/
		/* "frame1_1C" is the first frame with the known features.
		 * "frame2_1C" is the second frame where we want to find the first frame's features.
		 * "pyramid1" and "pyramid2" are workspace for the algorithm.
		 * "frame1_features" are the features from the first frame.
		 * "frame2_features" is the (outputted) locations of those features in the second frame.
		 * "number_of_features" is the number of features in the frame1_features array.
		 * "optical_flow_window" is the size of the window to use to avoid the aperture problem.
		 * "5" is the maximum number of pyramids to use.  0 would be just one level.
		 * "optical_flow_found_feature" is as described above (non-zero iff feature found by the flow).
		 * "optical_flow_feature_error" is as described above (error in the flow for this feature).
		 * "optical_flow_termination_criteria" is as described above (how long the algorithm should look).
		 * "0" means disable enhancements.  (For example, the second array isn't pre-initialized with guesses.)
		 */
		cvCalcOpticalFlowPyrLK(
			frame1_1C, 
			frame2_1C, 
			pyramid1, 
			pyramid2, 
			frame1_features, 
			frame2_features, 
			number_of_features, 
			optical_flow_window, 
			5, 
			optical_flow_found_feature, 
			optical_flow_feature_error, 
			optical_flow_termination_criteria, 
			0 );
	

		
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
