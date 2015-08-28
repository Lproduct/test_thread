#ifndef THREADIC_H
#define THREADIC_H

#include <QThread>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class ThreadIC : public QThread
{
    Q_OBJECT
public:
    explicit ThreadIC(QObject *parent = 0);
    void run();

    bool stop;

signals:
    void  imageChange(cv::Mat);

};

#endif // THREADIC_H
