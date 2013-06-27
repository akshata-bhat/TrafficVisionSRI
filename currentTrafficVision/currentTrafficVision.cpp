/*
 * Traffic Vision 1
 * This program will focus on grabing a video from a file 
 * and displaying it to the user.
 * 
 */


#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <math.h>
#include <iostream>


using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	
	VideoCapture cap("/home/trafficvision/Desktop/VideoFromFile/traffic.mp4"); // open the video file for reading

    if ( !cap.isOpened() )  // if not success, exit program
    {
         cout << "Cannot open the video file" << endl;
         return -1;
    }

	double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video

     cout << "Frame per seconds : " << fps << endl;
	
	
	/* Create a windows called "Traffic Vision Original" for visualizing the output.
	 * Have the window automatically change its size to match the output.
	 */
	namedWindow("Traffic Vision Original", CV_WINDOW_AUTOSIZE);
  
    while(1)
    {
		Mat frame;
        bool bSuccess = cap.read(frame); // read a new frame from video
        
		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read the frame from video file" << endl;
			break;
		}
	
	/* Now display the image we drew on.  
	 * Recall that "Traffic Vision Original" is the name of the window we created above.
	 */
	imshow("Traffic Vision Original", frame);
	
		if(waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break; 
		}
		
	}

    
    return 0; //Exit 
}

