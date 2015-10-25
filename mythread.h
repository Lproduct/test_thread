#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QtCore>
#include <QImage>
#include <Object.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <qmetatype.h>
#include "cvblob.h"
#include "particlecounting.h"

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    void run();

    bool stop;


public slots:
    void setAlpha(int newAlpha);
    void setBeta(int newBeta);
    void setH_MIN(int value);
    void setS_MIN(int value);
    void setV_MIN(int value);
    void setH_MAX(int value);
    void setS_MAX(int value);
    void setV_MAX(int value);
    void setVerticalLine();
    void setHorizontalLine();
    void setCalibrationMode(bool state);
    void setHSVobjMax(Scalar config);
    void setHSVobjMin(Scalar config);
    void setObjColor(Scalar config);
    void setObjType(std::string type);
    void setDeviceNumber(int number);
    void clearCount();
    void setSleepTime(unsigned long msSleep);
    void setAverageValue(int value);
    void setRecordingSate(bool state);
    void setrecordingDirPath(QString str);

signals:
    void imageChange(unsigned char*);
    void imageChangeCV(cv::Mat);
    void ballCoordinate(QString);
    void imageChangeThreshold(cv::Mat);
    void objNumber(int);
    void addObjectToCount();
    void countAB(int);
    void countBA(int);
    void count(Count);
    void timerVal(int);
    void timerControl(int);

private:
    cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true );
    cv::Mat imageProcessing(cv::Mat &image);
    void morphOps(cv::Mat &thresh);
    void trackFilteredObject(Object theObject,cv::Mat threshold,cv::Mat HSV, cv::Mat &cameraFeed);
    void trackFilteredObjectcvBlob(Object theObject, cv::Mat threshold, cv::Mat HSV, cv::Mat &cameraFeed);
    void drawObject(vector<Object> theObjects,cv::Mat &frame, cv::Mat &temp, vector< vector<Point> > contours, vector<Vec4i> hierarchy);
    string intToString(int number);

    QImage imageQt;
    QImage imageQtConvert;
    cv::Mat imageCv;
    cv::Mat imageCvShare;
    cv::Mat imageProcessed;

    cv::Mat src, src_gray;
    cv::Mat dst;

    //max number of objects to be detected in frame
    const int MAX_NUM_OBJECTS=50;
    //minimum and maximum object area
    const int MIN_OBJECT_AREA = 20*20;

    int alpha;
    int beta;

    int H_MIN;
    int S_MIN;
    int V_MIN;
    int H_MAX;
    int S_MAX;
    int V_MAX;

    Object objdetect;

    cvb::CvTracks tracks;
    ParticleCounting* partCount;

    bool calibrationMode;

    int deviceNumber;

    int sleepTime;
    int averageValue;

    bool recordingState;
    QString recordingDirPath;
};

#endif // MYTHREAD_H
