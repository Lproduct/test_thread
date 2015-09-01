#ifndef COUNTDISPLAY_H
#define COUNTDISPLAY_H

#include <QMainWindow>

namespace Ui {
class CountDisplay;
}

class CountDisplay : public QMainWindow
{
    Q_OBJECT

public:
    explicit CountDisplay(QWidget *parent = 0);
    ~CountDisplay();

private:
    Ui::CountDisplay *ui;
};

#endif // COUNTDISPLAY_H
