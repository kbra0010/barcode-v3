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
            //Mat temp = imread("C:/Users/Kurty/Pictures/barcode.png");
            //imshow("read", temp); // replace temp with thresholdedFrame
            detectBarcode(thresholdedFrame); // replace temp with thresholdedFrame
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
    // hardcoding BAD do int* pixel = new int[imgWidth] instead, just so debugger ain't shit

    
    int barcodeStartCol; //starting column of the barcode (first (left most) black region)
    bool barcodeStartFlag = false; //flag for determining whether the first column has been found or not

    int barcodeEndCol; //last column of the barcode (last (right most) black region)

    // getting pixel array
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

    int barcodeWidth = barcodeEndCol - barcodeStartCol;
    //int* finalBarcode = new int[barcodeWidth]; // final dynamic array of barcode as 0 or 255 values

    // finding digit 1 start bar
    int digitStartPixel[12]; //index is the digit number (0th being the first digit), specifies pixel at which that number starts
    // digitStartPixel[0] also defines the end of the left guard bar

    int T1pixels[12]; // start of 1st black bar to start of 2nd
    int T2pixels[12]; //   end of 1st black bar to   end of 2nd
    int T3pixels[12]; // start of 2nd black bar to end of char
    int T4pixels[12]; //   end of 2nd black bar to end of char
    int T4epixels[12]; // 
    int unitSize = 0; // size of a singular "1" bar
    // each of these arrays specify the T values for each digit index IN PIXELS

    // defining unit size
    // count from start of barcode (a 1 on the guard bar no matter the orientation) to the end of that 1
    // assume all bits are that long
    for (int i = barcodeStartCol; pixel[i] == 0; i++) { //keep incrementing i (column) until pixel goes white
        unitSize++;
    }



    ///*
    // define left guard bar
    int barCounter = 0;
    for (int col = barcodeStartCol+1; col <= barcodeEndCol; col++) { // +1 compensates for 0th pixel, so prev pixel is 0th pixel
        int previousPixel = pixel[col-1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel) {// to count transitions between white and black
            barCounter++;
        }
        // if they're equal then just increment col

        // if to break out if guard bar done
        if (barCounter == 3) {
            digitStartPixel[0] = col; // the first pixel of the 0th digit (first pixel within the first white / black space of that digit)
            break;
        }
    }

    // define right guard bar
    barCounter = 0;
    for (int col = barcodeEndCol - 1; col >= barcodeStartCol; col--) { // -1 compensates for rightmost pixel, so prev pixel is 1st from the right
        int previousPixel = pixel[col + 1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel) {// to count transitions between white and black
            barCounter++;
        }
        // if they're equal then just increment col

        // if to break out if guard bar done
        if (barCounter == 3) {
             // the last pixel of the 12th digit (last pixel within the last white / black space of that digit)
            break;
        }
    }

    // define central guard
    int centralGuardPixel = (barcodeStartCol + barcodeEndCol) / 2; // in the middle of a 0

    // seeking algos for central guard bar midpoint
    for (int col = centralGuardPixel; col < centralGuardPixel + 6; col++) {
        int previousPixel = pixel[col + 1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel && currentPixel == 255) {// to count transitions between white and black
            centralGuardPixel = col + 2;
        }
    }
    for (int col = centralGuardPixel; col > centralGuardPixel - 6; col--) {
        int previousPixel = pixel[col + 1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel && currentPixel == 255) {// to count transitions between white and black
            centralGuardPixel = col - 2;
        }
    }

    

    // reading T values from R to L from central guard bar
    int centralGuardBarRightPixel;
    int centralGuardBarLeftPixel;

    //finding left central guard bar edge
    barCounter = 0;
    for (int col = centralGuardPixel; col >= barcodeStartCol; col--) { // -1 compensates for rightmost pixel, so prev pixel is 1st from the right
        int previousPixel = pixel[col + 1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel) {// to count transitions between white and black
            barCounter++;
        }
        // if they're equal then just increment col

        // if to break out if guard bar done
        if (barCounter == 3) {
            centralGuardBarLeftPixel = col; // the last pixel of the 12th digit (last pixel within the last white / black space of that digit)
            break;
        }
    }

    //finding right central guard bar edge
    barCounter = 0;
    for (int col = centralGuardPixel; col <= barcodeEndCol; col++) { // -1 compensates for rightmost pixel, so prev pixel is 1st from the right
        int previousPixel = pixel[col - 1];
        int currentPixel = pixel[col];
        if (currentPixel != previousPixel) {// to count transitions between white and black
            barCounter++;
        }
        // if they're equal then just increment col

        // if to break out if guard bar done
        if (barCounter == 3) {
            centralGuardBarRightPixel = col; // the last pixel of the 12th digit (last pixel within the last white / black space of that digit)
            break;
        }
    }

    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    // everything above this is correct
    



    // get unitSize with central guard bar
    int centralUnitSize = 0;
    int i = centralGuardBarLeftPixel+1;
    while(pixel[i] == 255) { //keep incrementing i (column) until pixel goes black
        centralUnitSize++;
        i++;
    }






    // ----------------------------------------------------------------------------------------------------
    // finished guard bar definition
    // starting calculation of T values












    // loop for first digit from central guard bar (R to L), getting T values for it
    // regionStart[0]; black
    // regionStart[1]; black
    // regionStart[2]; black
    // regionStart[3]; black
    // colour always goes B, W, B ,W as measured R to L
    // test for pixel to the left after to determine colour
    int region = 0;
    int regionStart[4]; // 4 regions, 2 black 2 white
    int startingCol = centralGuardBarLeftPixel;
    for (int currentDigit = 5; currentDigit >= 0; currentDigit--) {
        // loop through digits from centre to LHS
        barCounter = 0;
        region = 0;

        

        for (int col = startingCol; barCounter < 4; col--) {
            int previousPixel = pixel[col + 1];
            int currentPixel = pixel[col];
            if (region == 0) { // finds which region the current digit starts in
                regionStart[0] = col;
                region++;
                continue;
            }
            else if (currentPixel != previousPixel) {// to count transitions between white and black
                regionStart[barCounter] = col;
                region++;
                barCounter++;
            }
        }
        startingCol += 7 * unitSize;
        // redefine unit size for accuracy?

        // regions definitions used to find T values
        // not accounting for small differences in which pixel is measured (to the L or R of barrier)
        T1pixels[currentDigit] = (regionStart[0] - regionStart[2]);
        T2pixels[currentDigit] = (regionStart[1] - regionStart[3]);
        T3pixels[currentDigit] = (regionStart[2] - (regionStart[0] - centralUnitSize * 7));
        T4pixels[currentDigit] = (regionStart[3] - (regionStart[0] - centralUnitSize * 7));
        // I'll do T4e later, it's a bit harder

    }
    // ABOVE LOOP IS PROBABLY DODGE
    
    





    // count the first 6 digits on the left
    // find the central guard bar
    // count the 6 digits on the right


    
    // need to find what 1 unit of width looks like
        // find minimum run of black pixels?


    // get T values
    // use T to work out characters and parity
    // obtain orientation
    // use to detet barcode orientation
    // set characters and parity
    //*/
    return;
}