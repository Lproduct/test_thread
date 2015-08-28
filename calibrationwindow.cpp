#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"

CalibrationWindow::CalibrationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CalibrationWindow)
{
    ui->setupUi(this);

    changeColorMax();
    changeColorMin();

    connect(ui->horizontalSliderH_Max, SIGNAL(valueChanged(int)), this, SLOT(changeColorMax()));
    connect(ui->horizontalSliderS_Max, SIGNAL(valueChanged(int)), this, SLOT(changeColorMax()));
    connect(ui->horizontalSliderV_Max, SIGNAL(valueChanged(int)), this, SLOT(changeColorMax()));
    connect(ui->horizontalSliderH_Min, SIGNAL(valueChanged(int)), this, SLOT(changeColorMin()));
    connect(ui->horizontalSliderS_Min, SIGNAL(valueChanged(int)), this, SLOT(changeColorMin()));
    connect(ui->horizontalSliderV_Min, SIGNAL(valueChanged(int)), this, SLOT(changeColorMin()));
    connect(ui->horizontalSliderAMax, SIGNAL(valueChanged(int)), this, SLOT(changeColorMax()));
    connect(ui->horizontalSliderAMin, SIGNAL(valueChanged(int)), this, SLOT(changeColorMin()));
}

CalibrationWindow::~CalibrationWindow()
{
    delete ui;
}

void CalibrationWindow::on_horizontalSliderH_Max_valueChanged(int value)
{
    emit newH_MAX(value);
}

void CalibrationWindow::on_horizontalSliderH_Min_valueChanged(int value)
{
    emit newH_MIN(value);
}

void CalibrationWindow::on_horizontalSliderS_Min_valueChanged(int value)
{
    emit newS_MIN(value);
}

void CalibrationWindow::on_horizontalSliderS_Max_valueChanged(int value)
{
    emit newS_MAX(value);
}

void CalibrationWindow::on_horizontalSliderV_Min_valueChanged(int value)
{
    emit newV_MIN(value);
}

void CalibrationWindow::on_horizontalSliderV_Max_valueChanged(int value)
{
    emit newV_MAX(value);
}

void CalibrationWindow::on_pushButtonQuit_clicked()
{
    emit calibQuit();
    this->close();
}

void CalibrationWindow::changeColorMax()
{
    QColor colorMax;
    colorMax.setHsv(ui->horizontalSliderH_Max->value(),
                    ui->horizontalSliderS_Max->value(),
                    ui->horizontalSliderV_Max->value(),
                    ui->horizontalSliderAMax->value());
    QPalette pMax;
    pMax.setColor(QPalette::Base, colorMax);
    ui->textEditColorMax->setPalette(pMax);
}

void CalibrationWindow::changeColorMin()
{
    QColor colorMin;
    colorMin.setHsv(ui->horizontalSliderH_Min->value(),
                    ui->horizontalSliderS_Min->value(),
                    ui->horizontalSliderV_Min->value(),
                    ui->horizontalSliderAMin->value());
    QPalette pMin;
    pMin.setColor(QPalette::Base, colorMin);
    ui->textEditColorMin->setPalette(pMin);
}
