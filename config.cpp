#include "config.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QVector>

Config::Config()
{
    file = "config.txt";
    getDataFromFile(file);
}

void Config::getDataFromFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString inTxt;
    QTextStream in(&file);
    inTxt = in.readAll();
    QVector<QString> data(inTxt.split(QRegExp("[\r\n]"),QString::SkipEmptyParts).toVector());

    for(int i(0); i< data.size(); i+=9)
    {
        DataConf configTemp;
        for (int j(i); j< i+9; j++)
        {
            QString temp((data[j].split(":").toVector()).at(0));
            if (temp == "Name")
            {
                configTemp.name = (data[j].split(":").toVector()).at(1);
            }
            else if(temp == "H_Max")
            {
                configTemp.H_Max = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "H_Min")
            {
                configTemp.H_Min = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "S_Max")
            {
                configTemp.S_Max = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "S_Min")
            {
                configTemp.S_Min = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "V_Max")
            {
                configTemp.V_Max = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "V_Min")
            {
                configTemp.V_Min = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "A_Max")
            {
                configTemp.A_Max = ((data[j].split(":").toVector()).at(1)).toInt();
            }
            else if(temp == "A_Min")
            {
                configTemp.A_Min = ((data[j].split(":").toVector()).at(1)).toInt();
            }
        }
        config.push_back(configTemp);
    }
}

QVector<DataConf> Config::getConf()
{
    return config;
}

void Config::updateConf()
{
    config.clear();
    getDataFromFile(file);
}


