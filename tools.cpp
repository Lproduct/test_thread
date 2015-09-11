#include "tools.h"
#include "ui_tools.h"

Tools::Tools(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Tools)
{
    ui->setupUi(this);
    connect(ui->pushButtonExit, SIGNAL(pressed()), this, SLOT(close()));

    ui->radioButtonH->setChecked(true);
    ui->radioButtonUpDown->setChecked(true);
    ui->radioButtonLeftRight->setChecked(true);
    ui->radioButtonLeftRight->setEnabled(false);
    ui->radioButtonRightLeft->setEnabled(false);
}

Tools::~Tools()
{
    delete ui;
}

void Tools::on_horizontalSliderAlpha_valueChanged(int value)
{
    emit alphaChanged(value);
}

void Tools::on_horizontalSliderBeta_valueChanged(int value)
{
    emit betaChanged(value);
}

void Tools::on_radioButtonH_clicked()
{
    ui->radioButtonLeftRight->setEnabled(false);
    ui->radioButtonRightLeft->setEnabled(false);
    ui->radioButtonUpDown->setEnabled(true);
    ui->radioButtonDownUp->setEnabled(true);
    emit horizontalLine();
}

void Tools::on_radioButtonV_clicked()
{
    ui->radioButtonLeftRight->setEnabled(true);
    ui->radioButtonRightLeft->setEnabled(true);
    ui->radioButtonUpDown->setEnabled(false);
    ui->radioButtonDownUp->setEnabled(false);
    emit verticalLine();
    emit clearCount();
}

void Tools::on_radioButtonUpDown_clicked()
{
    emit signalUpDown();
    emit clearCount();
}

void Tools::on_radioButtonDownUp_clicked()
{
    emit signalDownUp();
    emit clearCount();
}

void Tools::on_radioButtonLeftRight_clicked()
{
    emit signalLeftRight();
    emit clearCount();
}

void Tools::on_radioButtonRightLeft_clicked()
{
    emit signalRightLeft();
    emit clearCount();
}
