#include "widget.h"
#include "ui_widget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>
#include <algorithm>
#include <QMessageBox>
#include <iostream>
#include <cmath>
#include    <QFileDialog>
#include <QIODevice>
#include <QTextStream>

using namespace std;

void Widget::actSaveDataToFile()
{
    QString curPath=QDir::currentPath();
    QString dlgTitle="另存为一个文件";
    QString filter="文本文件(*.txt);;所有文件(*.*)";
    QString aFileName=QFileDialog::getSaveFileName(this,
                                 dlgTitle,curPath,filter);
    if(aFileName.isEmpty())
        return ;

    saveTextByIODevice(aFileName);
}

void Widget::saveTextByIODevice(const QString &aFileName)
{//用IODevice方式保存文本文件
    QFile aFile(aFileName);
    if(!aFile.open(QIODevice::WriteOnly | QIODevice::Text) ){
        qDebug()<<"保存到txt文件失败!";
        return ;
    }

    QString str="";
    QString curDate=curTime("yyyy-MM-dd");

    for(int i=0;i<dataPool3.size();i++){
        str+=curDate+" "+dataPool3[i].Time+"\t"+QString::number(dataPool3[i].Frequency)+
                "\t"+ QString::number(dataPool3[i].Temperature/10.0, 'f', 1)+"\n";
    }

    QByteArray strBytes=str.toUtf8();//转换为字节数组
    aFile.write(strBytes,strBytes.length());        //写入文件
    aFile.close();
    qDebug()<<"保存到txt文件成功!";
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("openQCM Wi2-Edt2022");

    iniLineChart();
    //图表主题初始化为2:Dark
    getSerialInfo();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnOpenSerial_clicked()
{
    if(ui->btnOpenSerial->text() == tr("打开串口"))
        {
            serial = new QSerialPort;

            QString port=ui->PortBox->currentText();
            QString baud=ui->BaudBox->currentText();
            QString bit=ui->BitBox->currentText();
            QString parity=ui->ParityBox->currentText();
            QString stop=ui->StopBox->currentText();

            qDebug()<<port<<","<<baud<<","<<bit<<","<<parity<<","<<stop;

            //设置串口名       
             serial->setPortName(port);

            //打卡串口
            serial->open(QIODevice::ReadWrite);
            //设置波特率          
            serial->setBaudRate(baud.toInt());
            //设置数据位数
            switch (bit.toInt())
            {
            case 8:
                serial->setDataBits(QSerialPort::Data8);
            case 7:
                serial->setDataBits(QSerialPort::Data7);
            case 6:
                serial->setDataBits(QSerialPort::Data6);
            case 5:
                serial->setDataBits(QSerialPort::Data5);
                break;
            default:
                break;
            }
            //设置校验位
            switch (ui->ParityBox->currentIndex())
            {
            case 0:
                serial->setParity(QSerialPort::NoParity);
                break;
            default:
                break;
            }
            //设置停止位
            switch (stop.toInt())
            {
            case 1:
                serial->setStopBits(QSerialPort::OneStop);
                break;
            case 2:
                serial->setStopBits(QSerialPort::TwoStop);
            default:
                break;
            }
            //设置流控制
            serial->setFlowControl(QSerialPort::NoFlowControl);

            //关闭设置菜单使能
            ui->PortBox->setEnabled(false);
            ui->BaudBox->setEnabled(false);
            ui->BitBox->setEnabled(false);
            ui->ParityBox->setEnabled(false);
            ui->StopBox->setEnabled(false);
            ui->lineEditMaxWindow->setEnabled(false);
            ui->btnOpenSerial->setText(tr("关闭串口"));

            //连接信号槽
            connect(serial,&QSerialPort::readyRead,this,&Widget::ReadData);
        }
        else
        {
            //关闭串口
            serial->clear();
            serial->close();
            serial->deleteLater();

            //恢复设置使能
            ui->PortBox->setEnabled(true);
            ui->BaudBox->setEnabled(true);
            ui->BitBox->setEnabled(true);
            ui->ParityBox->setEnabled(true);
            ui->StopBox->setEnabled(true);
            ui->lineEditMaxWindow->setEnabled(true);
            ui->btnOpenSerial->setText(tr("打开串口"));
        }

}

void Widget::ReadData()
{
    QString msg=curTime("hh:mm:ss")+" 读取数据: ";
    QByteArray buf;
        buf = serial->readAll();
//        qDebug()<<buf;
        if(!buf.isEmpty())
        {

            QString cur=tr(buf).toUtf8();
            QString word="RAWMONITOR";
            QString srcData=cur.mid(word.length(),cur.length()-word.length()-1);
           // qDebug()<<"srcData:"<<srcData;
            QStringList dataList=srcData.split("_");

            int freq=dataList[0].toInt();
            int temperature=dataList[1].toInt();

            dataPool3.push_back(Point3(freq,temperature,curTime("hh:mm:ss")));
            ui->labResonanceFreq->setText(QString::number(freq));
            ui->labTemperature->setText(QString::number(temperature/10.0));

//            qDebug()<<"freq:"<<freq<<",ampl:"<<ampl;
            msg+= cur;
            ui->plainTextEdit->appendPlainText(msg);

            buildChartV4(dataPool3);
        }
        buf.clear();
}

QString Widget::curTime(QString type)
{
    QDateTime curDateTime=QDateTime::currentDateTime();

    QString msg=curDateTime.toString(type);
    return msg;
}

void Widget::iniLineChart()
{//折线图初始化
    QChart *chart = new QChart(); //创建chart
//    chart->setTitle("Frequency/Temperature");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    ui->chartViewLine->setChart(chart); //为ChartView设置chart
    ui->chartViewLine->setRenderHint(QPainter::Antialiasing);
    ui->chartViewLine->chart()->setTheme(QChart::ChartTheme(2));

}

void Widget::buildChartV4(QVector<Point3> src)
{
//    QChart *chart = new QChart();
    QChart *chart = new QChart();

    chart->setTheme(QChart::ChartThemeDark);
    chart->setTitle("Real-Time slot:Frequency/Temperature");
    chart->setMargins(QMargins(10, 10, 10, 10));

    lineSeries0 = new QLineSeries();
    lineSeries1 = new QLineSeries();

    lineSeries0->setName("Frequency");
    lineSeries1->setName("Temperature");

    QColor color0,color1;

    color0.setRgb(0,255,255,255);
    color1.setRgb(255,215,0,255);


    lineSeries0->setColor(color0);
    lineSeries1->setColor(color1);

    int maxFq,minFq;
    int maxTp,minTp;
    maxFq=minFq=src[0].Frequency;
    maxTp=minTp=src[0].Temperature;
    int span=maxWindowSize;
    int start=0;
    int end=src.size();
    if(src.size()>span)
        start=src.size()-span;

//    qDebug()<<"src.size():"<<src.size();


    for(int i=start;i<end;i++){
        int freq=src[i].Frequency;
        int temperature=src[i].Temperature;

        maxFq=(src[i].Frequency>maxFq)?src[i].Frequency:maxFq;
        minFq=(src[i].Frequency<minFq)?src[i].Frequency:minFq;

        maxTp=(src[i].Temperature>maxTp)?src[i].Temperature:maxTp;
        minTp=(src[i].Temperature<minTp)?src[i].Temperature:minTp;

        int dfreq=freq;
        double dtemp=(double)temperature/10.0;

        lineSeries0->append(i,dfreq);
        lineSeries1->append(i,dtemp);

    }

    // 添加轴
//    QBarCategoryAxis *xAxis=new QBarCategoryAxis();

    QValueAxis *xAxis=new QValueAxis();
    QValueAxis *yAxis0 = new QValueAxis();
    QValueAxis *yAxis1 = new QValueAxis();

    maxFq+=25;
    minFq-=25;

    maxTp+=25;
    minTp-=25;

    //用于横坐标的字符串序列，
    QStringList categories;
    for(int i=start;i<end;i++)
        categories<<src[i].Time;

    //用于柱状图的横坐标轴

    //xAxis->append(categories);  //添加横坐标文字列表
//    chart->setAxisX(axisX,Line);//为折线图设置横坐标
    //xAxis->setRange(categories.at(0),categories.at(categories.count()-1));
    xAxis->setRange(start,end-1);
    xAxis->setGridLineVisible(false);
    xAxis->setTickCount(10);


    yAxis0->setRange(minFq, maxFq);
    yAxis1->setRange(minTp/10.0,maxTp/10.0);

    xAxis->setTitleText("Time(Ps)");

    yAxis0->setTickCount(13);
    yAxis0->setLabelFormat("%.0f");//标签格式
    yAxis0->setTitleText("Frequency(Hz)");

    yAxis1->setTickCount(13);
    yAxis1->setLabelFormat("%.1f");//标签格式
    yAxis1->setTitleText("Temperature(℃)");


    yAxis0->setTitleBrush(color0);
    yAxis1->setTitleBrush(color1);

    chart->addSeries(lineSeries0);
    chart->addSeries(lineSeries1);

    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis0, Qt::AlignLeft);
    chart->addAxis(yAxis1, Qt::AlignRight);

    chart->setAxisX(xAxis,lineSeries0);
    chart->setAxisY(yAxis0,lineSeries0);

    chart->setAxisX(xAxis,lineSeries1);
    chart->setAxisY(yAxis1,lineSeries1);

    lineSeries1->setVisible(showTemperature);

//    lineSeries0->attachAxis(xAxis);
//    lineSeries0->attachAxis(yAxis0);

//    lineSeries1->attachAxis(xAxis);
//    lineSeries1->attachAxis(yAxis1);
    ui->chartViewLine->chart()->removeAllSeries();
    ui->chartViewLine->chart()->removeAxis(ui->chartViewLine->chart()->axisX());
    ui->chartViewLine->chart()->removeAxis(ui->chartViewLine->chart()->axisY());

    ui->chartViewLine->setChart(chart);

    ui->chartViewLine->setRubberBand(QChartView::RectangleRubberBand);
    ui->chartViewLine->setRenderHint(QPainter::Antialiasing, true);
    //    ui->chartViewLine->chart()->setTheme(QChart::ChartTheme(2));
}

void Widget::getSerialInfo()
{//输出可用的串口的信息
    QString portInfo="检测到可用的串口的信息:\n";
    int cnt=1;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
//        qDebug() << "Name : " << info.portName();
//        qDebug() << "Description : " << info.description();
//        qDebug() << "Manufacturer: " << info.manufacturer();
//        qDebug() << "Serial Number: " << info.serialNumber();
//        qDebug() << "System Location: " << info.systemLocation();
        QString portName=info.portName();
        ui->PortBox->setCurrentText(portName);


        portInfo+="Port"+QString::number(cnt++)+"\n";
        portInfo+="Name : "+info.portName()+"\n";
        portInfo+="Description : "+info.description()+"\n";
        portInfo+="Manufacturer : "+info.manufacturer()+"\n";
        portInfo+="Serial Number : "+info.serialNumber()+"\n";
        portInfo+="System Location : "+info.systemLocation()+"\n";
    }
    ui->plainTextEdit->appendPlainText(portInfo);

}


void Widget::on_btnClearReception_clicked()
{
    ui->plainTextEdit->clear();

    QChart *chart=ui->chartViewLine->chart();//获取ChartView关联的chart

    chart->removeAllSeries();   //删除所有序列
    chart->removeAxis(chart->axisX());//删除坐标轴
    chart->removeAxis(chart->axisY());//删除坐标轴
    chart->removeAxis(chart->axisY());//删除坐标轴
    dataPool3.clear();

}

void Widget::on_btnSaveWindow_clicked()
{
    //保存读取到的数据到txt文件

    if(dataPool3.empty()){
        QMessageBox::warning(this,"Warning","尚未接收到数据，无法保存到文件！");
        return;
    }

    actSaveDataToFile();
}

void Widget::on_btnSaveToDB_clicked()
{
//    QMessageBox::information(this,"Info","Test:保存到数据库成功!");
}

void Widget::on_lineEditMaxWindow_textChanged(const QString &arg1)
{
    maxWindowSize=arg1.toInt();
    qDebug()<<"maxWindowSize:"<<maxWindowSize;
}

void Widget::on_btnShowTemperature_clicked()
{
    QString str=ui->btnShowTemperature->text();
    if(str=="show"){
        showTemperature=true;
        ui->btnShowTemperature->setText("hide");
    }else{
        showTemperature=false;
        ui->btnShowTemperature->setText("show");
    }
    lineSeries1->setVisible(showTemperature);
}

void Widget::on_btnCheckHistory_clicked()
{
    buildChartV4(dataPool3);
}
