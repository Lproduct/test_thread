#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"
#include <QInputDialog>
#include <QDir>
#include <QTextStream>
#include <QIODevice>
#include <QString>

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

    connect(ui->pushButtonSave, SIGNAL(pressed()), this, SLOT(saveConfig()));
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

void CalibrationWindow::saveConfig()
{
    bool ok;
    QString confName = QInputDialog::getText(this, "Configuration name",
                                             "Name:", QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);

    if (confName.isEmpty())
    {
        return;
    }

    QFile file("config.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString inTxt;
    QTextStream in(&file);
    inTxt = in.readAll();

    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

    QTextStream out(&file);
    out << inTxt;
    out << "Name:";
    out << confName;
    out << "\n";
    out << "H_Max:";
    out << QString::number(ui->horizontalSliderH_Max->value());
    out << "\n";
    out << "H_Min:";
    out << QString::number(ui->horizontalSliderH_Min->value());
    out << "\n";
    out << "S_Max:";
    out << QString::number(ui->horizontalSliderS_Max->value());
    out << "\n";
    out << "S_Min:";
    out << QString::number(ui->horizontalSliderS_Min->value());
    out << "\n";
    out << "V_Max:";
    out << QString::number(ui->horizontalSliderV_Max->value());
    out << "\n";
    out << "V_Min:";
    out << QString::number(ui->horizontalSliderV_Min->value());
    out << "\n";
    out << "A_Max:";
    out << QString::number(ui->horizontalSliderAMax->value());
    out << "\n";
    out << "A_Min:";
    out << QString::number(ui->horizontalSliderAMin->value());
    out << "\n";

    file.close();

    emit calibUpdate();
}
