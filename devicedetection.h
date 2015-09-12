#ifndef DeviceDetection_H
#define DeviceDetection_H

#include <QMainWindow>
#include <QString>
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <apps/Common/exampleHelper.h>

class DeviceDetection: public QMainWindow
{
    Q_OBJECT
public:
    explicit DeviceDetection(QWidget *parent = 0);
    ~DeviceDetection();
    void getDeviceFromUserInput();

signals:
    void warning(QString);
    void devices(QVector<QString>);
};

#endif // DeviceDetection_H
