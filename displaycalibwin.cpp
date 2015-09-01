#include "displaycalibwin.h"
#include "ui_displaycalibwin.h"
#include <QImage>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

DisplayCalibWin::DisplayCalibWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DisplayCalibWin)
{
    ui->setupUi(this);
}

DisplayCalibWin::~DisplayCalibWin()
{
    delete ui;
}

void DisplayCalibWin::displayImage(cv::Mat imageCV)
{
    qimgOriginal = cvMatToQImage(imageCV);
    ui->labelImageDisplay->setPixmap(QPixmap::fromImage(qimgOriginal));
    ui->labelImageDisplay->setScaledContents(true);
}

QImage DisplayCalibWin::cvMatToQImage( const cv::Mat &inMat )
   {
      switch ( inMat.type() )
      {
         // 8-bit, 4 channel
         case CV_8UC4:
         {
            QImage image(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGBX8888 );

            return image;
         }

         // 8-bit, 3 channel
         case CV_8UC3:
         {
            QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );

            return image.rgbSwapped();
         }

         // 8-bit, 1 channel
         case CV_8UC1:
         {
            static QVector<QRgb>  sColorTable;

            // only create our color table once
            if ( sColorTable.isEmpty() )
            {
               for ( int i = 0; i < 256; ++i )
                  sColorTable.push_back( qRgb( i, i, i ) );
            }

            QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );

            return image;
         }

         default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
      }

      return QImage();
   }
