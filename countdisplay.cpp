#include "countdisplay.h"
#include "ui_countdisplay.h"

CountDisplay::CountDisplay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CountDisplay)
{
    ui->setupUi(this);
}

CountDisplay::~CountDisplay()
{
    delete ui;
}
