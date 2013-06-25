/*
 * Traffic Vision 2
 * Display the original video.
 * Conduction background subtraction.
 * Display filtered video.
 * 
 */


#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;


class BackgroundSubtractor : public Algorithm
{
public:
    virtual ~BackgroundSubtractor();
    virtual void operator()(InputArray image, OutputArray fgmask, double learningRate=0);
    virtual void getBackgroundImage(OutputArray backgroundImage) const;
};

int main(int argc, char* argv[])
{
	
	// open the video file for reading
    VideoCapture cap("/home/trafficvision/Desktop/TrafficVision1/traffic.mp4"); 

    if ( !cap.isOpened() )  // if not success, exit program
    {
         cout << "Cannot open the video file" << endl;
         return -1;
    }

	//get the frames per seconds of the video
    double fps = cap.get(CV_CAP_PROP_FPS); 
     cout << "Frame per seconds : " << fps << endl;

	//create a window called "Traffic Vision Original"
    namedWindow("Traffic Vision Original",CV_WINDOW_AUTOSIZE); 
    

    while(1)
    {
        Mat frame;

        bool bSuccess = cap.read(frame); // read a new frame from video

		

if (!bSuccess) //if not success, break loop
{
                        cout << "Cannot read the frame from video file" << endl;
break;
}


        imshow("Traffic Vision Original", frame); //show the frame in "Original" window


        
        if(waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
{
                cout << "esc key is pressed by user" << endl;
break; 
}
    }

    return 0;

}

