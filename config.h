#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QVector>

struct DataConf
{
    QString name;
    int H_Max;
    int H_Min;
    int S_Max;
    int S_Min;
    int V_Max;
    int V_Min;
    int A_Max;
    int A_Min;
};

class Config
{
public:
    Config();
    QVector<DataConf> getConf();
    void updateConf();

private:
    void getDataFromFile(QString fileName);
    QVector<DataConf> config;
    QString file;
};

#endif // CONFIG_H
