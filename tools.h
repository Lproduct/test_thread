#ifndef TOOLS_H
#define TOOLS_H

#include <QMainWindow>

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

private slots:
    void on_horizontalSliderAlpha_valueChanged(int value);

    void on_horizontalSliderBeta_valueChanged(int value);

private:
    Ui::Tools *ui;
};

#endif // TOOLS_H
