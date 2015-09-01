#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QtCore>
#include <QImage>
#include <Object.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <qmetatype.h>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    void run();

    bool stop;
    int sleepTime;

public slots:
    void setAlpha(int newAlpha);
    void setBeta(int newBeta);
    void setH_MIN(int value);
    void setS_MIN(int value);
    void setV_MIN(int value);
    void setH_MAX(int value);
    void setS_MAX(int value);
    void setV_MAX(int value);
    void setCalibrationMode(bool state);
    void setHSVobjMax(Scalar config);
    void setHSVobjMin(Scalar config);
    void setObjColor(Scalar config);
    void setObjType(std::string type);

signals:
    void imageChange(unsigned char*);
    void imageChangeCV(cv::Mat);
    void ballCoordinate(QString);
    void imageChangeThreshold(cv::Mat);

private:
    cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true );
    cv::Mat imageProcessing(cv::Mat &image);
    void morphOps(cv::Mat &thresh);
    void trackFilteredObject(Object theObject,cv::Mat threshold,cv::Mat HSV, cv::Mat &cameraFeed);
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

    bool calibrationMode;
};

#endif // MYTHREAD_H
