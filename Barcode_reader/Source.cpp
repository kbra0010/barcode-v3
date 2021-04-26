#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp" //not sure if necessary
#include <opencv2/opencv.hpp> //required for thresholding
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

void detectBarcode(Mat inputImg);
string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch (depth) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
    }

    r += "C";
    r += (chans + '0');

    return r;
}

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
            detectBarcode(thresholdedFrame);
            break; // could put barcode check code here instead if you wanted
        }
    }

    
    // the camera will be deinitialized automatically in VideoCapture destructor
    
    system("pause");
    
    return 0;
}



void detectBarcode(Mat inputImgPointer) {
    // assuming input Mat has been thresholded
    // take middle line in image (horizontally)
    Mat inputImg = inputImgPointer.clone();
    int channels = inputImg.channels(); // should always be 1 (for thresholded / binary pixel images)
    int imgHeight = inputImg.size().height;
    int imgWidth = inputImg.size().width;
    int type = inputImg.type();
    String typeStr = type2str(type); // is generally of type CV_8UC1
    int halfwayRow = imgHeight / 2;
    //int* pixel = new int[imgWidth]; //initialising a dynamic array for the row of pixels along the image's horizontal centreline
    // this dynamic array works, but debugger doesn't understand it, so it only shows the first value of the array when debugging
    int pixel[640];
    // hardcoding BAD

    
    int barcodeStartCol; //starting column of the barcode (first (left most) black region)
    bool barcodeStartFlag = false; //flag for determining whether the first column has been found or not

    int barcodeEndCol; //last column of the barcode (last (right most) black region)

    for (int col = 0; col < imgWidth; col++) {
        pixel[col] = inputImg.at<uchar>(Point(col, halfwayRow)); // (row, col)
        if (barcodeStartFlag == false && pixel[col] == 0) {
            barcodeStartCol = col;
            barcodeStartFlag = true;
        }
        if (pixel[col] == 0) {
            barcodeEndCol = col;
        }
    }

    // count numbers in each bar
        // scan along until you see a black bar
    

    // get T values
    // use T to work out characters and parity
    // use to detet barcode orientation
    // set characters and parity
    return;
}