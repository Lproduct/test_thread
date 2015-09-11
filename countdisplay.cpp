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
    i = 0;
    countWayUDLR = true;
    connect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(on_horizontalSlider_valueChanged(int)));
    connect(ui->pushButtonNumColor, SIGNAL(pressed()), this, SLOT(on_pushButton_clicked()));
}

CountDisplay::~CountDisplay()
{
    delete ui;
}

void CountDisplay::valueChangeAB(int value)
{
    ui->lcdNumber->display(QString::number(value));
}

void CountDisplay::valueChangeBA(int value)
{
    ui->lcdNumber->display(QString::number(value));
}

void CountDisplay::valueChange(Count count)
{
    if(countWayUDLR == true)
    {
        ui->lcdNumber->display(QString::number(count.countAB));
    }
    else if(countWayUDLR == false)
    {
        ui->lcdNumber->display(QString::number(count.countBA));
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

void CountDisplay::setCountingObj()
{
    i++;
}
