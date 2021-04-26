#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp" //not sure if necessary
#include <opencv2/opencv.hpp> //required for thresholding
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int, char**) {
    Mat frame;
    Mat greyscaleFrame;
    Mat thresholdedFrame;

    // ------------------ INITIALIZE VIDEOCAPTURE ------------------
    VideoCapture cap;
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;

    // ------------------ MAIN PROGRAM LOOP ------------------
    while(true) {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // take frame and threshold it
        //thresholdedFrame = frame.clone();
        cvtColor(frame, greyscaleFrame, COLOR_RGB2GRAY);
        cv::threshold(greyscaleFrame, thresholdedFrame, 128, 255, THRESH_OTSU);
        //threshold(src, dst, thresh, maxValue, THRESH_BINARY);


        // show live and wait for a key with timeout long enough to show images
        //imshow("Live", frame);
        imshow("Greyscale", greyscaleFrame);
        imshow("Thresholded", thresholdedFrame);
        if (waitKey(5) >= 0) {
            break;
        }
    }

    imshow("poggers", thresholdedFrame);
    // the camera will be deinitialized automatically in VideoCapture destructor
    
    return 0;
}