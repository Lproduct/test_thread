#include "mythread.h"
#include <QThread>
#include <QDebug>

#include <QtCore>
#include <QImage>
#include <Object.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cvblob.h>

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

    objdetect.setColor(Scalar(0,0,0));
    objdetect.setHSVmax(Scalar(0,0,0));
    objdetect.setHSVmin(Scalar(0,0,0));

    partCount = new ParticleCounting();

    deviceNumber = 0;

    averageValue = 1;

    recordingState = false;
    recordingDirPath = "C:/SoFTraC_REC";
}

void MyThread::run()
{
    unsigned int cnt = 0;
    ////////////////////////    get an image from the camera //////////////////////////////
    //Get device
    DeviceManager devMgr;
    Device* pDev = getDeviceFromUserInput( devMgr , deviceNumber);

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
        QTime timer;
        timer.start();

        QTime timer_2;
        timer_2.start();

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

                imageCv.release();

                unsigned char* pTempBufQt = new unsigned char[pRequest->imageSize.read()];
                memcpy( pTempBufQt, pRequest->imageData.read(), pRequest->imageSize.read());
                //get QImage from camera image
                imageQt = QImage(pTempBufQt, pRequest->imageWidth.read(), pRequest->imageHeight.read(), QImage::Format_RGBX8888);

                //get cv::Mat from camera image
                imageQtConvert = imageQt.convertToFormat(QImage::Format_RGB32, Qt::ThresholdDither|Qt::AutoColor);
                imageCv = QImageToCvMat(imageQtConvert);

                for (int i(0); i< averageValue; i++)
                {
                    unsigned char* pTempBuf = new unsigned char[pRequest->imageSize.read()];
                    memcpy( pTempBuf, pRequest->imageData.read(), pRequest->imageSize.read());
                    //get QImage from camera image
                    QImage imageQtTmp;
                    imageQtTmp = QImage(pTempBuf, pRequest->imageWidth.read(), pRequest->imageHeight.read(), QImage::Format_RGBX8888);

                    //get cv::Mat from camera image
                    QImage imageQtConvertTmp;
                    imageQtConvertTmp = imageQtTmp.convertToFormat(QImage::Format_RGB32, Qt::ThresholdDither|Qt::AutoColor);
                    cv::Mat imageCvTmp;
                    imageCvTmp = QImageToCvMat(imageQtConvertTmp);

                    cv::addWeighted( imageCvTmp, 0.5, imageCv, 0.5, 0, imageCv);

                    delete pTempBuf;
                }

                cv::cvtColor(imageCv, imageCvShare, CV_RGB2BGR);

                imageProcessed = imageProcessing(imageCvShare);

                emit imageChangeCV(imageProcessed);
                delete pTempBufQt;

                /*
                //Copy data from camera image
                unsigned char* pTempBufQt = new unsigned char[pRequest->imageSize.read()];
                memcpy( pTempBufQt, pRequest->imageData.read(), pRequest->imageSize.read());

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
                */
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

        emit timerVal(timer.elapsed());
        this->msleep((int) sleepTime - timer.elapsed());

        emit timerControl(timer_2.elapsed());
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

    if(calibrationMode == true)
    {
        //need to find the appropriate color range values
        // calibrationMode must be false

        //if in calibration mode, we track objects based on the HSV slider values.
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);
        inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
        morphOps(threshold);

        emit imageChangeThreshold(threshold);

        //the folowing for canny edge detec
        /// Create a matrix of the same type and size as src (for dst)
        dst.create( src.size(), src.type() );
        /// Convert the image to grayscale
        cvtColor( src, src_gray, CV_BGR2GRAY );

        /// Show the image
        Object calib("calib");
        trackFilteredObject(calib,threshold,HSV,imageAB);
    }
    else
    {
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);

        //Object blue("blue");
        //first find blue objects
        cvtColor(imageAB,HSV,COLOR_BGR2HSV);
        inRange(HSV,objdetect.getHSVmin(),objdetect.getHSVmax(),threshold);
        morphOps(threshold);
        trackFilteredObject(objdetect,threshold,HSV,imageAB);
        //trackFilteredObjectcvBlob(objdetect,threshold,HSV,imageAB);

        //partCount->setInput(imageAB);
        //partCount->setTracks(tracks);
        //partCount->process();

        //emit count(partCount->getCount());
        //emit countAB(partCount->getCountAB());
        //emit countBA(partCount->getCountBA());
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

void MyThread::trackFilteredObjectcvBlob(Object theObject, cv::Mat threshold, cv::Mat HSV, cv::Mat &cameraFeed)
{
    int minArea = 10;
    int maxArea = 1000000;

    IplImage* frame = new IplImage(cameraFeed);
    cvConvertScale(frame, frame, 1, 0);

    IplImage* labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);
    IplImage* segmentated = new IplImage(threshold);

    cvb::CvBlobs blobs;
    unsigned int result = cvb::cvLabel(segmentated, labelImg, blobs);

    cvb::cvFilterByArea(blobs, minArea, maxArea);

    cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE|CV_BLOB_RENDER_TO_STD);

    cvb::cvUpdateTracks(blobs, tracks, 200., 5);

    cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_STD);

    cv::Mat img_result(frame);
    img_result.copyTo(cameraFeed);

    cvReleaseImage(&labelImg);
    delete frame;
    delete segmentated;
    cvReleaseBlobs(blobs);
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
        emit objNumber(numObjects);

        //record images if objects found
        if(recordingState == true)
        {
            QString dateTime(QDateTime::currentDateTime().toString("hh_mm_ss_zzz"));
            std::string filePath(recordingDirPath.toStdString() + "/Image_" + dateTime.toStdString() + ".jpg");
            if (QDir(recordingDirPath).exists() == false)
            {
                QDir().mkdir(recordingDirPath);
            }
            cv::imwrite(filePath , cameraFeed);
        }
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

                for (int i(0); i<objects.size(); i++)
                {
                    if(objects.at(i).getYPos() < 600 && objects.at(i).getXPos() > 550)
                    {
                        emit addObjectToCount();
                    }
                }
            }
        }
        else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0,50), 1, 2, Scalar(0,0,255), 2);
    }
    else if (hierarchy.size() == 0)
    {
        emit objNumber(0);
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

void MyThread::setHSVobjMax(Scalar config)
{
    QMutex mutex;
    mutex.lock();
    objdetect.setHSVmax(config);
    mutex.unlock();
}

void MyThread::setHSVobjMin(Scalar config)
{
    QMutex mutex;
    mutex.lock();
    objdetect.setHSVmin(config);
    mutex.unlock();
}

void MyThread::setObjColor(Scalar config)
{
    QMutex mutex;
    mutex.lock();
    objdetect.setColor(config);
    mutex.unlock();
}

void MyThread::setObjType(std::string type)
{
    QMutex mutex;
    mutex.lock();
    objdetect.setType(type);
    mutex.unlock();
}

void MyThread::setVerticalLine()
{
    QMutex mutex;
    mutex.lock();
    partCount->setLineVertical();
    mutex.unlock();
}

void MyThread::setHorizontalLine()
{
    QMutex mutex;
    mutex.lock();
    partCount->setLineHorizontal();
    mutex.unlock();
}

void MyThread::clearCount()
{
    QMutex mutex;
    mutex.lock();
    partCount->clearCount();
    mutex.unlock();
}

void MyThread::setDeviceNumber(int number)
{
    QMutex mutex;
    mutex.lock();
    deviceNumber = number;
    mutex.unlock();
}

void MyThread::setSleepTime(unsigned long msSleep)
{
    QMutex mutex;
    mutex.lock();
    sleepTime = msSleep;
    mutex.unlock();
}

void MyThread::setAverageValue(int value)
{
    QMutex mutex;
    mutex.lock();
    averageValue = value;
    mutex.unlock();
}

void MyThread::setRecordingSate(bool state)
{
    QMutex mutex;
    mutex.lock();
    recordingState = state;
    mutex.unlock();
}

void MyThread::setrecordingDirPath(QString str)
{
    QMutex mutex;
    mutex.lock();
    recordingDirPath = str;
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


