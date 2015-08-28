#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <QMainWindow>

namespace Ui {
class CalibrationWindow;
}

class CalibrationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CalibrationWindow(QWidget *parent = 0);
    ~CalibrationWindow();

signals:
    void newH_MIN(int);
    void newV_MIN(int);
    void newS_MIN(int);
    void newH_MAX(int);
    void newV_MAX(int);
    void newS_MAX(int);
    void calibQuit();

private slots:
    void on_horizontalSliderH_Max_valueChanged(int value);
    void on_horizontalSliderH_Min_valueChanged(int value);
    void on_horizontalSliderS_Min_valueChanged(int value);
    void on_horizontalSliderS_Max_valueChanged(int value);
    void on_horizontalSliderV_Min_valueChanged(int value);
    void on_horizontalSliderV_Max_valueChanged(int value);
    void on_pushButtonQuit_clicked();
    void changeColorMax();
    void changeColorMin();

private:
    Ui::CalibrationWindow *ui;
};

#endif // CALIBRATIONWINDOW_H
