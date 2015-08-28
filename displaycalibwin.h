#ifndef DISPLAYCALIBWIN_H
#define DISPLAYCALIBWIN_H

#include <QMainWindow>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Ui {
class DisplayCalibWin;
}

class DisplayCalibWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit DisplayCalibWin(QWidget *parent = 0);
    ~DisplayCalibWin();

public slots:
    void displayImage(cv::Mat imageCV);

private:
    Ui::DisplayCalibWin *ui;
    QImage cvMatToQImage( const cv::Mat &inMat );

    cv::Mat qimgOriginal;
};

#endif // DISPLAYCALIBWIN_H
