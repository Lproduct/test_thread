#include "tools.h"
#include "ui_tools.h"

Tools::Tools(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Tools)
{
    ui->setupUi(this);
    connect(ui->pushButtonExit, SIGNAL(pressed()), this, SLOT(close()));
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
