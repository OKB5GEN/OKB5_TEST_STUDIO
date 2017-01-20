#include "OTD.h"
#include "mainwindow.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QString>
#include "qapplication.h"
#include <QtSerialPort/QtSerialPort>
#include "synchapi.h"

QSerialPort *com7;
QByteArray bw;
QString data,temp;
QByteArray readData0;
int len1=0, len2=0;
QTimer *MyTimerOTD;
int flag_otd=1;

OTD::OTD(QString s) : name(s)
{

}

void OTD::COMConnectorOTD()
{
    com7 = new QSerialPort("com7");
    com7->open(QIODevice::ReadWrite);
    com7->setBaudRate(QSerialPort::Baud115200);
    com7->setDataBits(QSerialPort::Data8);
    com7->setParity(QSerialPort::NoParity);
    com7->setStopBits(QSerialPort::OneStop);
    com7->setFlowControl(QSerialPort::NoFlowControl);
    OTD_id();
    OTDtemper();

}
void OTD::OTD_id()
{
    if(flag_otd==1){
        bw.resize(4);
        bw[0] = 0xff;
        bw[1] = 0x01;
        bw[2] = 0x00;
        bw[3] = 0x01;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        QByteArray readData1 = com7->readAll();
        while (com7->waitForReadyRead(100))
            readData1.append(com7->readAll());

        bw[0] = 0xff;
        bw[1] = 0x01;
        bw[2] = 0x00;
        bw[3] = 0x02;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        readData0 = com7->readAll();
        while (com7->waitForReadyRead(500))
            readData0.append(com7->readAll());
        if(readData1[2]!=readData0[2] || readData1[3]!=readData0[3])
            emit OTD_id1();
    }
}
void OTD::OTD_req(){
    if(flag_otd==1){
        QString res;
        bw.resize(4);
        bw[0] = 0x44;
        bw[1] = 0x02;
        bw[2] = 0x00;
        bw[3] = 0x00;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        QByteArray readData1 = com7->readAll();
        while (com7->waitForReadyRead(100))
            readData1.append(com7->readAll());
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        res="";
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1==0)res+="ОТД: Модуль не готов к работе! \n";
        if(z2==1)res+="ОТД: Ошибки у модуля! \n";
        if(z3==1)res+="ОТД: Модуль после перезагрузки! \n";
        emit OTD_reqr (res);
    }
}
void OTD::OTD_fw()
{
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x06;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    emit OTD_vfw(readData1[2]*10+readData1[3]);
}
void OTD::echo_OTD()
{
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x07;
    bw[2] = 0xaa;
    bw[3] = 0xaa;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    if(readData1[3]==0xaa && readData1[2]==0xaa)
        emit echo(1);
    else emit echo (0);
}
void OTD::res_OTD()
{
    flag_otd=0;
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x04;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    COMCloseOTD();
    for(int i=0;i<500;i++){
        Sleep(10);
        QApplication::processEvents();
    }
    COMConnectorOTD();
    flag_otd=1;
    emit OTD_res(readData1[3]);
}
void OTD::err_res_OTD()
{
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x03;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    emit OTD_err_res(readData1[3]);
}
void OTD::OTDres1()
{
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x26;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(500))
        readData1.append(com7->readAll());
    if(readData1[3]==2)emit err_OTD("ОТД: Ошибка при перезагрузке!\n");
}
void OTD::OTDres2()
{
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x27;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(500))
        readData1.append(com7->readAll());
    if(readData1[3]==2)emit err_OTD("ОТД: Ошибка при перезагрузке!\n");
    else emit err_OTD("");
}
void OTD::OTDmeas1()
{
    if(flag_otd==1){
        bw.resize(4);
        bw[0] = 0x44;
        bw[1] = 0x28;
        bw[2] = 0x00;
        bw[3] = 0x00;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        Sleep(2000);
        QByteArray readData1 = com7->readAll();
        while (com7->waitForReadyRead(500))
            readData1.append(com7->readAll());
        if(readData1[3]==2)emit err_OTD("ОТД: Ошибка при запуске измерений 1-й линии!\n");
        OTDtm1();
    }
}
void OTD::OTDmeas2()
{
    if(flag_otd==1){
        bw.resize(4);
        bw[0] = 0x44;
        bw[1] = 0x29;
        bw[2] = 0x00;
        bw[3] = 0x00;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        Sleep(2000);
        QByteArray readData1 = com7->readAll();
        while (com7->waitForReadyRead(500))
            readData1.append(com7->readAll());
        if(readData1[3]==2)emit err_OTD("ОТД: Ошибка при запуске измерений 2-й линии!\n");
        OTDtm2();
    }
}
void OTD::OTDtemper()
{
    if(flag_otd==1){
        data="";
        data+=" Кол-во датчиков DS1820 по оси 1: ";
        bw.resize(4);
        bw[0] = 0x44;
        bw[1] = 0x1d;
        bw[2] = 0x00;
        bw[3] = 0x00;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        readData0 = com7->readAll();
        while (com7->waitForReadyRead(500))
            readData0.append(com7->readAll());
        len1=readData0[2];
        data+=QString::number(len1);
        data+="\n";
        data+=" Адреса датчиков по оси 1: \n";
        for(int j=1;j<=len1;j++)
        {
            data+=QString::number(j);
            data+=" : ";
            for(int k=0;k<8;k++)
            {
                bw[0] = 0x44;
                bw[1] = 0x2a;
                bw[2] = j;
                bw[3] = k;
                com7->QIODevice::write(bw);
                com7->waitForBytesWritten(-1);
                QByteArray readData3 = com7->readAll();
                while (com7->waitForReadyRead(100))
                    readData3.append(com7->readAll());
                data+=QString::number(readData3[2],16);
            }
            data+= "\n";
        }
        data+="\n";
        if(readData0[3]==2)data+="ОТД: Ошибка при считывании датчиков 1-й оси\n";

        data+=" Кол-во датчиков DS1820 по оси 2: ";
        bw.resize(4);
        bw[0] = 0x44;
        bw[1] = 0x1e;
        bw[2] = 0x00;
        bw[3] = 0x00;
        com7->QIODevice::write(bw);
        com7->waitForBytesWritten(-1);
        QByteArray readData3 = com7->readAll();
        while (com7->waitForReadyRead(500))
            readData3.append(com7->readAll());
        len2=readData3[2];
        data+=QString::number(len2);
        data+="\n";
        data+=" Адреса датчиков по оси 2: \n";
        for(int j=1;j<=len2;j++)
        {
            data+=QString::number(j);
            data+=" : ";
            for(int k=0;k<8;k++)
            {
                bw[0] = 0x44;
                bw[1] = 0x2b;
                bw[2] = j;
                bw[3] = k;
                com7->QIODevice::write(bw);
                com7->waitForBytesWritten(-1);
                QByteArray readData4 = com7->readAll();
                while (com7->waitForReadyRead(100))
                    readData4.append(com7->readAll());
                data+=QString::number(readData4[2],16);
            }
            data+= "\n";
        }
        if(readData3[3]==2)data+="ОТД: Ошибка при считывании датчиков 2-й оси\n";
        emit temp_OTD(data);
    }

}
void OTD::OTDtm1()
{
    if(flag_otd==1){
        temp="";
        for(int i=1;i<=len1;i++){
            temp+=" Температура датчиков 1-й линии \n ";
            temp+=QString::number(i);
            temp+=" : ";
            bw.resize(4);
            bw[0] = 0x44;
            bw[1] = 0x1f;
            bw[2] = i;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData1 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData1.append(com7->readAll());
            uint8_t uu1,uu2;
            uu1=readData1[2];
            uu2=readData1[3];
            double uu=(uu1<<8) | uu2;
            uint8_t x=readData1[2],z;
            z=x<<4;
            z=z>>7;
            if(z==0)uu=uu/16;
            if(z==1)uu=(uu-4096)/16;
            if(uu>80) emit tm_OTD_err();
            temp+=QString::number(uu);
            temp+="\n";
        }
        emit tm_OTD1(temp);
    }
}
void OTD::OTDtm2()
{
    if(flag_otd==1){
        temp="";
        for(int i=1;i<=len2;i++){
            temp+=" Температура датчиков 2-й линии \n ";
            temp+=QString::number(i);
            temp+=" : ";
            bw.resize(4);
            bw[0] = 0x44;
            bw[1] = 0x20;
            bw[2] = i;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData1 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData1.append(com7->readAll());
            uint8_t uu1,uu2;
            uu1=readData1[2];
            uu2=readData1[3];
            double uu=(uu1<<8) | uu2;
            uint8_t x=readData1[2],z;
            z=x<<4;
            z=z>>7;
            if(z==0)uu=uu/16;
            if(z==1)uu=(uu-4096)/16;
            if(uu>80) emit tm_OTD_err();
            temp+=QString::number(uu);
            temp+="\n";
        }
        emit tm_OTD2(temp);
    }
}
void OTD::OTDPT(int x)
{
    if(flag_otd==1){
        double uu3,uu;
        if(x==1||x==3){
            bw.resize(4);
            bw[0] = 0x44;
            bw[1] = 0x1c;
            bw[2] = 0x01;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData1 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData1.append(com7->readAll());
            uint8_t uu1,uu2;
            uu1=readData1[2];
            uu2=readData1[3];
            uu=(uu1<<8) | uu2;
            uu=uu/32-256;
        }
        if(x==2||x==3){
            bw[0] = 0x44;
            bw[1] = 0x1c;
            bw[2] = 0x02;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData2 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData2.append(com7->readAll());
            uint8_t uu1,uu2;
            uu1=readData2[2];
            uu2=readData2[3];
            uu3=(uu1<<8) | uu2;
            uu3=uu3/32-256;

        }
        emit start_OTDPT(uu*100, uu3*100);
    }
}

void OTD::OTD_avt(int x, int y)
{
    if(x==1){
        MyTimerOTD = new QTimer;
        MyTimerOTD->start(y);
        QObject::connect(MyTimerOTD,SIGNAL(timeout()), this, SLOT( OTD_timer()));
    }
    else
    {
        y=0;
        QObject::disconnect(MyTimerOTD,SIGNAL(timeout()), this, SLOT( OTD_timer()));
    }
}
void OTD::OTD_timer()
{
    OTDPT(3);
    OTDmeas1();
    OTDmeas2();
}

void OTD:: COMCloseOTD(){
    com7->close();
}

void OTD::doWork()
{
    emit start_OTD();
}
