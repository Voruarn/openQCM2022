#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QtCharts>
#include <QMap>
using namespace QtCharts;

struct Point3{  //横坐标为时间，纵坐标为频率 和 温度
    int Frequency;//频率
    int Temperature;//温度
    QString Time;//MM-dd hh:mm:ss

    Point3(){}
    Point3(int f,int tp,QString t):Frequency(f),Temperature(tp),Time(t){}
};

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

private:
    QSerialPort *serial;

    QLineSeries *lineSeries0=nullptr;//用于显示读取到的传感器数据
    QLineSeries *lineSeries1=nullptr;//用于显示读取到的传感器数据

    QVector<Point3> dataPool3;

    bool showTemperature=true;

    int maxWindowSize=120;

    void actSaveDataToFile();

    void saveTextByIODevice(const QString &aFileName);

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void ReadData();

    QString curTime(QString type);

    void iniLineChart();     //折线图初始化

    void buildChartV4(QVector<Point3> src); //构建折线图

    void getSerialInfo();

private slots:
    void on_btnOpenSerial_clicked();

    void on_btnClearReception_clicked();

    void on_btnSaveWindow_clicked();

    void on_btnSaveToDB_clicked();

    void on_lineEditMaxWindow_textChanged(const QString &arg1);

    void on_btnShowTemperature_clicked();

    void on_btnCheckHistory_clicked();

private:
    Ui::Widget *ui;

};

#endif // WIDGET_H
