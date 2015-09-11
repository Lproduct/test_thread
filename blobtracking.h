#ifndef BLOBTRACKING_H
#define BLOBTRACKING_H

#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "cvblob.h"

class BlobTracking
{
public:
    BlobTracking();
    ~BlobTracking();

    void process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output);
    const cvb::CvTracks getTracks();

private:
    bool firstTime;
    int minArea;
    int maxArea;

    bool debugTrack;
    bool debugBlob;
    bool showBlobMask;
    bool showOutput;

    cvb::CvTracks tracks;
};

#endif // BLOBTRACKING_H
