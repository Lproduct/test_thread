#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "qmetatype.h"
#include "calibrationwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    hLine = true;

    connect(ui->pushButtonGetImage, SIGNAL(pressed()), this, SLOT(on_pushButtonGetImage_clicked()));
    connect(ui->pushButtonStop, SIGNAL(pressed()), this, SLOT(on_pushButton_clicked()));

    mThread = new MyThread(this);
    qRegisterMetaType< cv::Mat >("cv::Mat");
    connect(mThread, SIGNAL(imageChangeCV(cv::Mat)), this, SLOT(onImageChangedCV(cv::Mat)));
    connect(mThread, SIGNAL(ballCoordinate(QString)), this, SLOT(setTextBallCoordinate(QString)));

    disCalWin = new DisplayCalibWin();
    connect(mThread, SIGNAL(imageChangeThreshold(cv::Mat)), disCalWin, SLOT(displayImage(cv::Mat)));

    calibWin = new CalibrationWindow(disCalWin);
    connect(calibWin, SIGNAL(newH_MAX(int)), mThread, SLOT(setH_MAX(int)));
    connect(calibWin, SIGNAL(newH_MIN(int)), mThread, SLOT(setH_MIN(int)));
    connect(calibWin, SIGNAL(newS_MAX(int)), mThread, SLOT(setS_MAX(int)));
    connect(calibWin, SIGNAL(newS_MIN(int)), mThread, SLOT(setS_MIN(int)));
    connect(calibWin, SIGNAL(newV_MAX(int)), mThread, SLOT(setV_MAX(int)));
    connect(calibWin, SIGNAL(newV_MIN(int)), mThread, SLOT(setV_MIN(int)));
    connect(calibWin, SIGNAL(calibQuit()), this, SLOT(winCalibQuit()));

    confData = new Config();
    setComboBoxConfInit();
    connect(calibWin, SIGNAL(calibUpdate()), this, SLOT(setComboBoxConf()));

    setConfigObjDetect(0);
    connect(ui->comboBoxObjectConf, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigObjDetect(int)));

    toolsWin = new Tools(this);
    connect(ui->pushButtonTools, SIGNAL(clicked(bool)), toolsWin, SLOT(show()));
    connect(toolsWin, SIGNAL(alphaChanged(int)), mThread, SLOT(setAlpha(int)));
    connect(toolsWin, SIGNAL(betaChanged(int)), mThread, SLOT(setBeta(int)));
    connect(toolsWin, SIGNAL(verticalLine()), mThread, SLOT(setVerticalLine()));
    connect(toolsWin, SIGNAL(horizontalLine()), mThread, SLOT(setHorizontalLine()));

    cntDisplay = new CountDisplay(this);
    connect(ui->pushButtonCount, SIGNAL(pressed()), cntDisplay, SLOT(show()));
    //connect(mThread, SIGNAL(objNumber(int)), cntDisplay, SLOT(valueChange(int)));
    //connect(mThread, SIGNAL(addObjectToCount()), cntDisplay, SLOT(setCountingObj()));
    connect(mThread, SIGNAL(countAB(int)), cntDisplay, SLOT(valueChangeAB(int)));
    connect(mThread, SIGNAL(countBA(int)), cntDisplay, SLOT(valueChangeBA(int)));
    qRegisterMetaType< Count >("Count");
    connect(mThread, SIGNAL(count(Count)), cntDisplay, SLOT(valueChange(Count)));
    connect(toolsWin, SIGNAL(signalLeftRight()), cntDisplay, SLOT(countUDLR()));
    connect(toolsWin, SIGNAL(signalUpDown()), cntDisplay, SLOT(countUDLR()));
    connect(toolsWin, SIGNAL(signalDownUp()), cntDisplay, SLOT(countDURL()));
    connect(toolsWin, SIGNAL(signalRightLeft()), cntDisplay, SLOT(countDURL()));
    connect(toolsWin, SIGNAL(clearCount()), mThread, SLOT(clearCount()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setComboBoxConfInit()
{
    confData->updateConf();
    //add new items
    QVector<DataConf> data(confData->getConf());
    for (int j(0); j< data.size() ;j++)
    {
        ui->comboBoxObjectConf->addItem(data.at(j).name);
    }
}

void MainWindow::setComboBoxConf()
{
    confData->updateConf();
    QVector<DataConf> data(confData->getConf());
    ui->comboBoxObjectConf->addItem(data.at(data.size()-1).name);
}

void MainWindow::setConfigObjDetect(int index)
{
    confData->updateConf();
    QVector<DataConf> data(confData->getConf());
    mThread->setHSVobjMax(Scalar(data.at(index).H_Max, data.at(index).S_Max, data.at(index).V_Max));
    mThread->setHSVobjMin(Scalar(data.at(index).H_Min, data.at(index).S_Min, data.at(index).V_Min));
    mThread->setObjType(data.at(index).name.toStdString());
    mThread->setObjColor(Scalar(0,255,255));
}

void MainWindow::onImageChangedCV(cv::Mat imageCV)
{
    qimgOriginal = cvMatToQImage(imageCV);
    ui->labelCameraViewopenCV->setPixmap(QPixmap::fromImage(qimgOriginal));
    ui->labelCameraViewopenCV->setScaledContents(true);
}

void MainWindow::on_pushButtonGetImage_clicked()
{
    mThread->sleepTime = 0;
    mThread->start();
}

void MainWindow::on_pushButton_clicked()
{
    mThread->stop = true;
}

////////////////////////////    convert QImage to cv::Mat and cv::Mat to QImage   ////////////////////////////
// If inImage exists for the lifetime of the resulting cv::Mat, pass false to inCloneImageData to share inImage's
// data with the cv::Mat directly
//    NOTE: Format_RGB888 is an exception since we need to use a local QImage and thus must clone the data regardless
cv::Mat MainWindow::QImageToCvMat(const QImage &inImage, bool inCloneImageData)
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

QImage MainWindow::cvMatToQImage( const cv::Mat &inMat )
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
////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setTextBallCoordinate(QString txt)
{
    //ui->plainTextEditBallCoord->appendPlainText(txt);
}

void MainWindow::on_pushButtonCalib_clicked()
{
    disCalWin->show();
    calibWin->show();
    mThread->setCalibrationMode(true);
}

void MainWindow::winCalibQuit()
{
    disCalWin->close();
    mThread->setCalibrationMode(false);
}
