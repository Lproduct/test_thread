#ifndef TOOLS_H
#define TOOLS_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>

namespace Ui {
class Tools;
}

class Tools : public QMainWindow
{
    Q_OBJECT

public:
    explicit Tools(QWidget *parent = 0);
    ~Tools();

signals:
    void alphaChanged(int value);
    void betaChanged(int value);
    void horizontalLine();
    void verticalLine();
    void signalUpDown();
    void signalDownUp();
    void signalRightLeft();
    void signalLeftRight();
    void clearCount();
    void speedTime(unsigned long value);
    void imageAverageSignal(int value);
    void fileRecPath(QString str);

private slots:
    void on_horizontalSliderAlpha_valueChanged(int value);
    void on_horizontalSliderBeta_valueChanged(int value);
    void on_radioButtonH_clicked();
    void on_radioButtonV_clicked();
    void on_radioButtonUpDown_clicked();
    void on_radioButtonDownUp_clicked();
    void on_radioButtonLeftRight_clicked();
    void on_radioButtonRightLeft_clicked();
    void speedTimeSetting(const double &value);
    void imageAverage(const int &value);

    void on_checkBoxEnableTRackCount_clicked(bool checked);
    void filePath();

private:
    Ui::Tools *ui;
};

#endif // TOOLS_H
