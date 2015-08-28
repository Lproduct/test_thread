#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#include "mythread.h"
#include "calibrationwindow.h"
#include "displaycalibwin.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MyThread* mThread;
    CalibrationWindow* calibWin;
    DisplayCalibWin* disCalWin;

public slots:
    void onImageChangedCV(cv::Mat imageCV);

private slots:
    void on_pushButtonGetImage_clicked();
    void on_pushButton_clicked();
    void spinBoxalpha_valueChanged();
    void spinBoxbeta_valueChanged();
    void setTextBallCoordinate(QString txt);
    void on_pushButtonCalib_clicked();
    void winCalibQuit();

signals:

private:
    Ui::MainWindow *ui;

    QImage imageQt;
    QImage imageQtC;
    QImage imageQtConvert;
    QImage qimgOriginal;

    cv::Mat imageCv;
    cv::Mat imageCvH;
    cv::Mat matOriginal;                    // original image
    cv::Mat matProcessed;                   // processed image
    QImage qimgProcessed;

    cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true );
    QImage cvMatToQImage( const cv::Mat &inMat );
};

#endif // MAINWINDOW_H