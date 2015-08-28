#include "mythread.h"
#include <QThread>
#include <QDebug>

#include <QtCore>
#include <QImage>
#include <Object.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef _MSC_VER // is Microsoft compiler?
#   if _MSC_VER < 1300  // is 'old' VC 6 compiler?
#       pragma warning( disable : 4786 ) // 'identifier was truncated to '255' characters in the debug information'
#   endif // #if _MSC_VER < 1300
#endif // #ifdef _MSC_VER
#include <iostream>
#include <Apps/Common/exampleHelper.h>
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#ifdef _WIN32
#   include <mvDisplay/Include/mvIMPACT_acquire_display.h>
using namespace mvIMPACT::acquire::display;
#endif // #ifdef _WIN32
using namespace mvIMPACT::acquire;
using namespace std;
#if defined(linux) || defined(__linux) || defined(__linux__)
#   define NO_DISPLAY
#else
#   undef NO_DISPLAY
#endif // #if defined(linux) || defined(__linux) || defined(__linux__)

typedef bool( *SUPPORTED_DEVICE_CHECK )( const mvIMPACT::acquire::Device* const );

MyThread::MyThread(QObject *parent) : QThread(parent)
{
    stop = false;
    alpha = 10;
    beta = 0;
    calibrationMode = false;

    H_MIN = 0;
    S_MIN = 0;
    V_MIN = 0;
    H_MAX = 0;
    S_MAX = 0;
    V_MAX = 0;
    namedWindow( "Thresholded Image", WINDOW_AUTOSIZE );
}

void MyThread::run()
{
    unsigned int cnt = 0;
    ////////////////////////    get an image from the camera //////////////////////////////
    //Get device
    DeviceManager devMgr;
    Device* pDev = getDeviceFromUserInput( devMgr );

    //get image from camera
    if( !pDev )
    {
        cout << "Unable to continue!" << endl
             << "Press [ENTER] to end the application" << endl;
        cin.get();
        //return 0;
    }

    try
    {
        pDev->open();
    }
    catch( const ImpactAcquireException& e )
    {
        // this e.g. might happen if the same device is already opened in another process...
        cout << "An error occurred while opening the device(error code: " << e.getErrorCode() << ")." << endl
             << "Press [ENTER] to end the application" << endl;
        cin.get();
        //return 0;
    }

    FunctionInterface fi( pDev );

    // Send all requests to the capture queue. There can be more than 1 queue for some device, but for this sample
    // we will work with the default capture queue. If a device supports more than one capture or result
    // queue, this will be stated in the manual. If nothing is set about it, the device supports one
    // queue only. This loop will send all requests currently available to the driver. To modify the number of requests
    // use the property mvIMPACT::acquire::SystemSettings::requestCount at runtime or the property
    // mvIMPACT::acquire::Device::defaultRequestCount BEFORE opening the device.
    TDMR_ERROR result = DMR_NO_ERROR;
    while( ( result = static_cast<TDMR_ERROR>( fi.imageRequestSingle() ) ) == DMR_NO_ERROR ) {};
    if( result != DEV_NO_FREE_REQUEST_AVAILABLE )
    {
        cout << "'FunctionInterface.imageRequestSingle' returned with an unexpected result: " << result
             << "(" << mvIMPACT::acquire::ImpactAcquireException::getErrorCodeAsString( result ) << ")" << endl;
    }
    manuallyStartAcquisitionIfNeeded( pDev, fi );
    // run thread loop
    Request* pRequest = 0;
    // we always have to keep at least 2 images as the displayWindow module might want to repaint the image, thus we
    // can free it unless we have a assigned the displayWindow to a new buffer.
    Request* pPreviousRequest = 0;
    const unsigned int timeout_ms = 500;
    while( !stop )
    {
        // wait for results from the default capture queue
        int requestNr = fi.imageRequestWaitFor( timeout_ms );
        pRequest = fi.isRequestNrValid( requestNr ) ? fi.getRequest( requestNr ) : 0;
        if( pRequest )
        {
            if( pRequest->isOK() )
            {
                ++cnt;
                // here we can displayWindow some statistical information every 100th image
                if( cnt % 100 == 0 )
                {
                    /*cout << "Info from "   << ": " << statistics.framesPerSecond.name() << ": " << statistics.framesPerSecond.readS()
                         << ", " << statistics.errorCount.name() << ": " << statistics.errorCount.readS()
                         << ", " << statistics.captureTime_s.name() << ": " << statistics.captureTime_s.readS() << endl;*/
                }

                //Copy data from camera image
                unsigned char* pTempBufQt = new unsigned char[pRequest->imageSize.read()];
                memcpy( pTempBufQt, pRequest->imageData.read(), pRequest->imageSize.read() );

                //get QImage from camera image
                imageQt = QImage(pTempBufQt, pRequest->imageWidth.read(), pRequest->imageHeight.read(), QImage::Format_RGBX8888);

                //get cv::Mat from camera image
                imageQtConvert = imageQt.convertToFormat(QImage::Format_RGB32, Qt::ThresholdDither|Qt::AutoColor);
                imageCv = QImageToCvMat(imageQtConvert);

                QMutex mutex;
                mutex.lock();
                cv::cvtColor(imageCv, imageCvShare, CV_RGB2BGR);
                mutex.unlock();

                imageProcessed = imageProcessing(imageCvShare);

                emit imageChangeCV(imageProcessed);
                this->msleep(sleepTime);
                delete pTempBufQt;
            }
            else
            {
                cout << "Error: " << pRequest->requestResult.readS() << endl;
            }
            if( pPreviousRequest )
            {
                // this image has been displayed thus the buffer is no longer needed...
                pPreviousRequest->unlock();
            }
            pPreviousRequest = pRequest;
            // send a new image request into the capture queue
            fi.imageRequestSingle();
        }
        else
        {
            // If the error code is -2119(DEV_WAIT_FOR_REQUEST_FAILED), the documentation will provide
            // additional information under TDMR_ERROR in the interface reference
            cout << "imageRequestWaitFor failed (" << requestNr << ", " << ImpactAcquireException::getErrorCodeAsString( requestNr ) << ")"
                 << ", timeout value too small?" << endl;
        }
    }
    manuallyStopAcquisitionIfNeeded( pDev, fi );

    // In this sample all the next lines are redundant as the device driver will be
    // closed now, but in a real world application a thread like this might be started
    // several times an then it becomes crucial to clean up correctly.
    // free the last potential locked request
    if( pRequest )
    {
        pRequest->unlock();
    }
    // clear the request queue
    fi.imageRequestReset( 0, 0 );
    // extract and unlock all requests that are now returned as 'aborted'
    int requestNr = INVALID_ID;
    while( ( requestNr = fi.imageRequestWaitFor( 0 ) ) >= 0 )
    {
        pRequest = fi.getRequest( requestNr );
        cout << "Request " << requestNr << " did return with status " << pRequest->requestResult.readS() << endl;
        pRequest->unlock();
    }
}

cv::Mat MyThread::imageProcessing(cv::Mat &image)
{
    cv::Mat imageAB;
    Mat threshold;
    Mat HSV;

    image.convertTo(imageAB, -1, alpha, beta);

    src = imageAB;

    /////////////////Algo 1//////////////
/*
    cv::Mat matProcessed;
    cv::GaussianBlur(imageAB, matProcessed, cv::Size(5, 5), 2.0);

    cv::inRange(matProcessed, cv::Scalar(0, 0, 175), cv::Scalar(100, 100, 256), matProcessed);

    cv::GaussianBlur(matProcessed, matProcessed, cv::Size(5, 5), 2.0);

    cv::dilate(matProcessed, matProcessed, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));		// close image (dilate, then erode)
    cv::erode(matProcessed, matProcessed, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));		// closing "closes" (i.e. fills in) foreground gaps

    std::vector<cv::Vec3f> v3fCircles;                                                                              // declare circles variable
    cv::HoughCircles(matProcessed, v3fCircles, CV_HOUGH_GRADIENT, 2, matProcessed.rows / 4, 100, 50, 10, 400);      // fill variable circles with all circles in the processed image

    for(unsigned int i = 0; i < v3fCircles.size(); i++)
    {                                                                                       // for each circle
        emit ballCoordinate(QString("ball position x =") + QString::number(v3fCircles[i][0]).rightJustified(4, ' ') +              // print ball position and radius
                                                                         QString(", y =") + QString::number(v3fCircles[i][1]).rightJustified(4, ' ') +
                                                                         QString(", radius =") + QString::number(v3fCircles[i][2], 'f', 3).rightJustified(7, ' '));

        cv::circle(imageAB, cv::Point((int)v3fCircles[i][0], (int)v3fCircles[i][1]), 3, cv::Scalar(0, 255, 0), CV_FILLED);                  // draw small green circle at center of detected object
        cv::circle(imageAB, cv::Point((int)v3fCircles[i][0], (int)v3fCircles[i][1]), (int)v3fCircles[i][2], cv::Scalar(0, 0, 255), 3);      // draw red circle around the detected object
    }
*/
    /////////////////////////////////////

   /////////////////Algo 2///////////////
/*
    //Convert to HSV
    CvSize size = cvGetSize(imageAB);
    IplImage *hsv = cvCreateImage(size, IPL_DEPTH_8U, 3);
    cvCvtColor(imageAB, hsv, CV_BGR2HSV);
    //return hsv;

    //Generate mask
    CvMat *mask = cvCreateMat(size.height, size.width, CV_8UC1);
    cvInRangeS(hsv, cvScalar(0.11*256, 0.60*256, 0.20*256, 0),
                    cvScalar(0.14*256, 1.00*256, 1.00*256, 0), mask);
    cvReleaseImage(&hsv);
    //IplImage *tmp = cvCreateImage(size, 8, 1);
    //cvCopy(mask, tmp, NULL);
    //return mask;

    //Perform morphological ops
    IplConvKernel *se21 = cvCreateStructuringElementEx(21, 21, 10, 10, CV_SHAPE_RECT, NULL);
    IplConvKernel *se11 = cvCreateStructuringElementEx(11, 11, 5,  5,  CV_SHAPE_RECT, NULL);
    cvClose(mask, mask, se21);
    cvOpen(mask, mask, se11);
    cvReleaseStructuringElement(&se21);
    cvReleaseStructuringElement(&se11);
    //IplImage *tmp = cvCreateImage(size, 8, 1);
    //cvCopy(mask, tmp, NULL);
    //return mask;

    //Hough transform
    IplImage *hough_in = cvCreateImage(size, 8, 1);
    cvCopy(mask, hough_in, NULL);

    print_time("Finding hough circles");
    CvMemStorage *storage = cvCreateMemStorage(0);
        cvSmooth(hough_in, hough_in, CV_GAUSSIAN, 15, 15, 0, 0);
    CvSeq *circles = cvHoughCircles(
        hough_in, storage, // input, storage,
        CV_HOUGH_GRADIENT, 4, size.height/10,
                           // type, 1/scale, min center dists
        100, 40,           // params1?, param2?
        0, 0               // min radius, max radius
    );
    cvReleaseMemStorage(&storage);

    //Fancy up output
    print_time("Generating output");
    int i;
    for (i = 0; i < circles->total; i++) {
             float *p = (float*)cvGetSeqElem(circles, i);
         CvPoint center = cvPoint(cvRound(p[0]),cvRound(p[1]));
         CvScalar val = cvGet2D(mask, center.y, center.x);
         if (val.val[0] < 1) continue;
             cvCircle(img,  center, 3,             CV_RGB(0,255,0), -1, CV_AA, 0);
             cvCircle(img,  center, cvRound(p[2]), CV_RGB(255,0,0),  3, CV_AA, 0);
             cvCircle(mask, center, 3,             CV_RGB(0,255,0), -1, CV_AA, 0);
             cvCircle(mask, center, cvRound(p[2]), CV_RGB(255,0,0),  3, CV_AA, 0);
    }
*/
    /////////////////////////////////////

    ////////////////Algo 3///////////////

    if(calibrationMode == true)
    {
        //need to find the appropriate color range values
        // calibrationMode must be false

        //if in calibration mode, we track objects based on the HSV slider values.
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);
        inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
        morphOps(threshold);
        imshow("Thresholded Image",threshold);

        //the folowing for canny edge detec
        /// Create a matrix of the same type and size as src (for dst)
        dst.create( src.size(), src.type() );
        /// Convert the image to grayscale
        cvtColor( src, src_gray, CV_BGR2GRAY );

        /// Show the image
        Object none("none");
        trackFilteredObject(none,threshold,HSV,imageAB);
    }
    else
    {
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);

        Object blue("blue");
        //first find blue objects
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);
        inRange(HSV,blue.getHSVmin(),blue.getHSVmax(),threshold);
        morphOps(threshold);
        trackFilteredObject(blue,threshold,HSV,imageAB);
    }

    /////////////////////////////////////

    return imageAB;
}

void MyThread::morphOps(cv::Mat &thresh)
{
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

    erode(thresh,thresh,erodeElement);
    erode(thresh,thresh,erodeElement);

    dilate(thresh,thresh,dilateElement);
    dilate(thresh,thresh,dilateElement);
}

void MyThread::trackFilteredObject(Object theObject, cv::Mat threshold, cv::Mat HSV, cv::Mat &cameraFeed)
{

    vector <Object> objects;
    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0)
    {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects < MAX_NUM_OBJECTS)
        {
            for (int index = 0; index >= 0; index = hierarchy[index][0])
            {
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;

                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area > MIN_OBJECT_AREA)
                {
                    Object object;

                    object.setXPos(moment.m10/area);
                    object.setYPos(moment.m01/area);
                    object.setType(theObject.getType());
                    object.setColor(theObject.getColor());

                    objects.push_back(object);

                    objectFound = true;
                }

                else objectFound = false;
            }
            //let user know you found an object
            if(objectFound ==true)
            {
                //draw object location on screen
                drawObject(objects,cameraFeed,temp,contours,hierarchy);
            }
        }
        else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0,50), 1, 2, Scalar(0,0,255), 2);
    }
}

void MyThread::drawObject(vector<Object> theObjects,cv::Mat &frame, cv::Mat &temp, vector< vector<Point> > contours, vector<Vec4i> hierarchy)
{
    for(int i =0; i<theObjects.size(); i++)
    {
        cv::drawContours(frame,contours,i,theObjects.at(i).getColor(),3,8,hierarchy);
        cv::circle(frame,cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()),5,theObjects.at(i).getColor());
        cv::putText(frame,intToString(theObjects.at(i).getXPos())+ " , " + intToString(theObjects.at(i).getYPos()),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()+20),1,1,theObjects.at(i).getColor());
        cv::putText(frame,theObjects.at(i).getType(),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()-20),1,2,theObjects.at(i).getColor());
    }
}

string MyThread::intToString(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void MyThread::setAlpha(int newAlpha)
{
    QMutex mutex;
    mutex.lock();
    alpha = newAlpha;
    mutex.unlock();
}

void MyThread::setBeta(int newBeta)
{
    QMutex mutex;
    mutex.lock();
    beta = newBeta;
    mutex.unlock();
}

void MyThread::setH_MIN(int value)
{
    QMutex mutex;
    mutex.lock();
    H_MIN = value;
    mutex.unlock();
}

void MyThread::setS_MIN(int value)
{
    QMutex mutex;
    mutex.lock();
    S_MIN = value;
    mutex.unlock();
}

void MyThread::setV_MIN(int value)
{
    QMutex mutex;
    mutex.lock();
    V_MIN = value;
    mutex.unlock();
}

void MyThread::setH_MAX(int value)
{
    QMutex mutex;
    mutex.lock();
    H_MAX = value;
    mutex.unlock();
}

void MyThread::setS_MAX(int value)
{
    QMutex mutex;
    mutex.lock();
    S_MAX = value;
    mutex.unlock();
}

void MyThread::setV_MAX(int value)
{
    QMutex mutex;
    mutex.lock();
    V_MAX = value;
    mutex.unlock();
}

void MyThread::setCalibrationMode(bool state)
{
    QMutex mutex;
    mutex.lock();
    calibrationMode = state;
    mutex.unlock();
}

cv::Mat MyThread::QImageToCvMat(const QImage &inImage, bool inCloneImageData)
{
   switch ( inImage.format() )
   {
      // 8-bit, 4 channel
      case QImage::Format_RGB32:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         return (inCloneImageData ? mat.clone() : mat);
      }

      // 8-bit, 3 channel
      case QImage::Format_RGB888:
      {
         if ( !inCloneImageData )
            qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

         QImage   swapped = inImage.rgbSwapped();

         return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
      }

      // 8-bit, 1 channel
      case QImage::Format_Indexed8:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         return (inCloneImageData ? mat.clone() : mat);
      }

      default:
         qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
         break;
   }

   return cv::Mat();
}


