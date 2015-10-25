#include "countdisplay.h"
#include "ui_countdisplay.h"
#include <QString>
#include <QColorDialog>
#include <QPalette>

CountDisplay::CountDisplay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CountDisplay)
{
    ui->setupUi(this);
    objNb = 0;
    countWayUDLR = true;
    startCount = false;
    connect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(on_horizontalSlider_valueChanged(int)));
    connect(ui->pushButtonNumColor, SIGNAL(pressed()), this, SLOT(on_pushButton_clicked()));
    connect(ui->pushButtonPlayCount, SIGNAL(pressed()), this, SLOT(startCounting()));
    connect(ui->pushButtonPauseCount, SIGNAL(pressed()), this, SLOT(pauseCounting()));
    connect(ui->pushButtonRefreshCount, SIGNAL(pressed()), this, SLOT(restartCounting()));
    connect(ui->pushButtonRec, SIGNAL(clicked(bool)), this, SLOT(record(bool)));
}

CountDisplay::~CountDisplay()
{
    delete ui;
}

void CountDisplay::valueChangeAB(int value)
{
    //ui->lcdNumber->display(QString::number(value));
}

void CountDisplay::valueChangeBA(int value)
{
    //ui->lcdNumber->display(QString::number(value));
}

void CountDisplay::valueChange(Count count)
{
    if(countWayUDLR == true)
    {
        //ui->lcdNumber->display(QString::number(count.countAB));
    }
    else if(countWayUDLR == false)
    {
        //ui->lcdNumber->display(QString::number(count.countBA));
    }
}

void CountDisplay::on_horizontalSlider_valueChanged(int value)
{
    this->setWindowOpacity(((double)value)/10);
}

void CountDisplay::on_pushButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::black, this );
    QPalette p;
    p.setColor(QPalette::WindowText, color);
    ui->lcdNumber->setPalette(p);
}

void CountDisplay::countUDLR()
{
    countWayUDLR = true;
}

void CountDisplay::countDURL()
{
    countWayUDLR = false;
}

void CountDisplay::startCounting()
{
    startCount = true;
}

void CountDisplay::pauseCounting()
{
    startCount = false;
}

void CountDisplay::restartCounting()
{
    objNb = 0;
}

void CountDisplay::countObj(int obj)
{
    ui->lcdNumber_2->display(QString::number(obj));

    if (startCount == true)
    {
        objNb += obj;
    }
    else {}

    ui->lcdNumber->display(QString::number(objNb));
}

void CountDisplay::timerDisplay(int val)
{
    ui->labelTimer->setText(QString::number(val));
}

void CountDisplay::timerControlDisplay(int val)
{
    ui->labelTimerControl->setText(QString::number(val));
}

void CountDisplay::record(bool state)
{
    if(state == true)
    {
        emit recordState(true);
        ui->pushButtonRec->setChecked(true);
    }

    else if(state == false)
    {
        emit recordState(false);
        ui->pushButtonRec->setChecked(false);
    }
}
