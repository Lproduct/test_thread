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
public slots:
    void valueChangeAB(int value);
    void valueChangeBA(int value);
    void valueChange(Count count);
    void setCountingObj();
    void countUDLR();
    void countDURL();

private slots:
    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_clicked();

private:
    Ui::CountDisplay *ui;
    int i;
    bool countWayUDLR;
};

#endif // COUNTDISPLAY_H
