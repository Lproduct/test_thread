#ifndef COUNTDISPLAY_H
#define COUNTDISPLAY_H

#include <QMainWindow>
#include "particlecounting.h"

namespace Ui {
class CountDisplay;
}

class CountDisplay : public QMainWindow
{
    Q_OBJECT

public:
    explicit CountDisplay(QWidget *parent = 0);
    ~CountDisplay();

signals:
    void recordState(bool);

public slots:
    void valueChangeAB(int value);
    void valueChangeBA(int value);
    void valueChange(Count count);
    void startCounting();
    void pauseCounting();
    void restartCounting();
    void countUDLR();
    void countDURL();
    void countObj(int obj);
    void timerDisplay(int val);
    void timerControlDisplay(int val);
    void record(bool state);

private slots:
    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_clicked();

private:
    Ui::CountDisplay *ui;
    bool countWayUDLR;
    bool startCount;
    int objNb;
};

#endif // COUNTDISPLAY_H
