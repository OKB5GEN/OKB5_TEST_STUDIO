#include "Tech.h"
#include "mainwindow.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QObject>
#include <QString>
#include "qapplication.h"
#include <QtSerialPort/QtSerialPort>
#include "synchapi.h"


QSerialPort *com8, *com9, *com2;
QTimer *MyTimerTech;
QTime time;
time_t rawtime;
QString name;
struct tm * timeinfo;
int flag_Tech=1,flag_SSI=0, flag_log=0, flag_rm_41=0, prev_rm=0,flag_res_tech1=0,flag_res_tech=1;
double skvt_V=0,skvt_H=0;
double rm1=0,rm2=0;

Tech::Tech(QString s) : name(s)
{

}

void Tech::COMConnectorSKVT()
{
    com9 = new QSerialPort("com9");
    com9->open(QIODevice::ReadWrite);
    com9->setBaudRate(QSerialPort::Baud57600);
    com9->setDataBits(QSerialPort::Data8);
    com9->setParity(QSerialPort::NoParity);
    com9->setStopBits(QSerialPort::OneStop);
    com9->setFlowControl(QSerialPort::NoFlowControl);
}
void Tech::COMConnectorRM_2()
{
    com2 = new QSerialPort("com2");
    com2->open(QIODevice::ReadWrite);
    com2->setBaudRate(QSerialPort::Baud115200);
    com2->setDataBits(QSerialPort::Data8);
    com2->setParity(QSerialPort::NoParity);
    com2->setStopBits(QSerialPort::OneStop);
    com2->setFlowControl(QSerialPort::NoFlowControl);
}
void Tech::COMConnectorTech()
{
    com8 = new QSerialPort("com8");
    com8->open(QIODevice::ReadWrite);
    com8->setBaudRate(QSerialPort::Baud115200);
    com8->setDataBits(QSerialPort::Data8);
    com8->setParity(QSerialPort::NoParity);
    com8->setStopBits(QSerialPort::OneStop);
    com8->setFlowControl(QSerialPort::NoFlowControl);
    COMConnectorSKVT();
    COMConnectorRM_2();
}
int Tech::id_tech()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x01;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData1 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData1.append(com8->readAll());

    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x02;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData2.append(com8->readAll());
    if(readData1[2]==readData2[2]&&readData1[3]==readData2[3]) return 1;
    else return 0;
}
int Tech::res_err_tech()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x03;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData1 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData1.append(com8->readAll());
    if(readData1[3]!=1)emit tech_err(100);
    else emit tech_err(0);
    return 1;
}
int Tech::echo_tech()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x07;
    ba[2] = 0xaa;
    ba[3] = 0xaa;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData1 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData1.append(com8->readAll());
    if(readData1[3]!=0xaa || readData1[2]!=0xaa)
        emit tech_err(300);
    else emit tech_err(333);
    return 1;
}
int Tech::res_tech()
{
    QByteArray ba;
    flag_res_tech1=0;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x04;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData1 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData1.append(com8->readAll());
    COMCloseTech ();
    for(int i=0; i<500; i++) {
        Sleep(10);
        QApplication::processEvents();
    }
    COMConnectorTech();
    flag_res_tech1=1;
    if(readData1[3]!=1)emit tech_err(200);
    else emit tech_err(0);
    return 1;
}
QString Tech::tech_read_buf(int x,int len)
{
    QByteArray ba;
    int y=0;
    if(x==1) y=26;
    else y=20;
    QString result;
    for(int i=0; i<len; i++) {
        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = y;
        ba[2] = 0x00;
        ba[3] = 0x00;
        com8->QIODevice::write(ba);
        com8->waitForBytesWritten(-1);
        QByteArray readData2 = com8->readAll();
        while (com8->waitForReadyRead(100))
            readData2.append(com8->readAll());
        if(readData2[2]==1) emit tech_err(700);
        else if(readData2[2]==2) emit tech_err(750);
        else emit tech_err(0);
        result+=readData2[3];
        result+=" ";
        QApplication::processEvents();
    }
    if (x==2)emit tech_buf(result);
    if (x==1)emit tech_buf1(result);
    return result;
}
int Tech::tech_send(int com, int x, int y)
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = com;
    ba[2] = x;
    ba[3] = y;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData2.append(com8->readAll());
    if(readData2[3]!=1&& (com==27||com==21)&&(x==1||x==2)) emit tech_err(400);
    else if(readData2[3]!=1&& (com==27||com==21)&& x==3) emit tech_err(450);
    else if(readData2[3]!=1&& com==36) emit tech_err(500);
    else if(readData2[3]!=1&& com==37) emit tech_err(550);
    else if(readData2[3]!=1&& (com==17||com==23)) emit tech_err(600);
    else if(readData2[3]!=1&& (com==18||com==24)) emit tech_err(650);
    else emit tech_err(0);
    return 1;
}
int Tech::tech_read(int x)
{
    QByteArray ba;
    if(flag_res_tech==1) {
        int y=0;
        if(x==1) y=25;
        else y=19;
        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = y;
        ba[2] = 0x00;
        ba[3] = 0x00;
        com8->QIODevice::write(ba);
        com8->waitForBytesWritten(-1);
        QByteArray readData2 = com8->readAll();
        while (com8->waitForReadyRead(100))
            readData2.append(com8->readAll());
        uint8_t uu1,uu2;
        uu1=readData2[2];
        uu2=readData2[3];
        double uu=(uu1<<8) | uu2;

        if (uu != 0) {
            tech_read_buf (2, uu);
        }
    }
    return 0;
}
QString Tech::req_tech()
{
    QByteArray ba;
    if(flag_res_tech==1) {
        QString res;
        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = 0x02;
        ba[2] = 0x00;
        ba[3] = 0x00;
        com8->QIODevice::write(ba);
        com8->waitForBytesWritten(-1);
        QByteArray readData1 = com8->readAll();
        while (com8->waitForReadyRead(100))
            readData1.append(com8->readAll());
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1==0) res+=" Технол. модуль не готов к работе! \n";
        if(z2==1) res+=" Ошибки у Технол. модуля! \n";
        if(z3==1) res+=" Модуль Технол. после перезагрузки! \n";
        if(readData1[3]==0x10) res+=" Потеря байта из-за переполнения буфера RS485! \n";
        return res;
    }
    return "";
}
int Tech::fw_tech()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x06;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData1 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData1.append(com8->readAll());
    emit tech_err(readData1[2]*10+readData1[3]);
    return 1;
}
int tech_on_RM_2(int x)
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x66;
    ba[1] = 0x3b;
    ba[2] = x;
    ba[3] = 0x00;
    com2->QIODevice::write(ba);
    com2->waitForBytesWritten(-1);
    QByteArray readData2 = com2->readAll();
    while (com2->waitForReadyRead(100))
        readData2.append(com2->readAll());
    uint8_t uu;
    uu=readData2[3];
    return uu;
}
int tech_check_RM_2()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x66;
    ba[1] = 0x3c;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com2->QIODevice::write(ba);
    com2->waitForBytesWritten(-1);
    QByteArray readData2 = com2->readAll();
    while (com2->waitForReadyRead(100))
        readData2.append(com2->readAll());
    if(readData2[3]==1 && readData2[2]==1)
        return 1;
    return 0;
}
double tech_read_RM_2()
{
    QByteArray ba;
    QByteArray bb;
    ba.resize(4);
    ba[0] = 0x66;
    ba[1] = 0x3d;
    ba[2] = 0x00;
    ba[3] = 0x00;
    bb.resize(4);
    bb[0] = 0x56;
    bb[1] = 0x3d;
    bb[2] = 0x00;
    bb[3] = 0x00;
    com2->QIODevice::write(ba);
    com8->QIODevice::write(bb);
    com2->waitForBytesWritten(-1);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com2->readAll();
    QByteArray readData3 = com8->readAll();
    while (com2->waitForReadyRead(10))
        readData2.append(com2->readAll());

    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    uint8_t uu3,uu4;
    uu3=readData2[2];
    uu4=readData2[3];
    rm2=(uu3<<8) | uu4;
    uint8_t uu1,uu2;
    uu1=readData3[2];
    uu2=readData3[3];
    rm1=(uu1<<8) | uu2;
    return 1;
}
int tech_on_SSI(int x)
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x3b;
    ba[2] = x;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData2.append(com8->readAll());
    uint8_t uu;
    uu=readData2[3];
    return uu;
}
int tech_check_SSI()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x3c;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com8->readAll();
    while (com8->waitForReadyRead(100))
        readData2.append(com8->readAll());
    if(readData2[3]==1 && readData2[2]==1)
        return 1;
    return 0;
}
double tech_read_SSI()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x3d;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    QByteArray readData2 = com8->readAll();
    while (com8->waitForReadyRead(10))
        readData2.append(com8->readAll());
    uint8_t uu1,uu2;
    uu1=readData2[2];
    uu2=readData2[3];
    double uu=(uu1<<8) | uu2;
    return uu;
}
void SKVT_on(int x)
{
    QByteArray ba;
    ba.resize(7);
    ba[0] = 0xab;
    ba[1] = 0xcd;
    ba[2] = 0x01;
    ba[3] = 0x26;
    ba[4] = x;
    ba[5] = 0x0a;
    ba[6] = 0x0d;
    com9->QIODevice::write(ba);
    com9->waitForBytesWritten(-1);
    QByteArray readData3 = com9->readAll();
    while (com9->waitForReadyRead(50))
        readData3.append(com9->readAll());
}
void Tech::Tech_ssi (int x)
{
    if(x==1||x==0){
        if(tech_on_SSI(x)==1&&tech_check_SSI()==1)
        {
            flag_SSI=1;
            SKVT_on(1);
            Sleep(1500);
            SKVT_on(2);
        }
        else flag_SSI=0;
    }
    else if (x==2){
        if(tech_on_SSI(1)==1&&tech_check_SSI()==1&&tech_on_RM_2(1)==1&&tech_check_RM_2()==1)
        {
            flag_SSI=2;
        }
        else flag_SSI=0;
    }
    else if (x==3){
        if(tech_on_SSI(1)==1&&tech_check_SSI()==1)
        {
            SKVT_on(1);
            Sleep(1500);
            SKVT_on(2);
            flag_SSI=3;
        }
        else flag_SSI=0;
    }
    else if (x==4){
        tech_on_SSI(1);
    }
    else if (x==5){
        tech_on_SSI(0);
    }
}
void Tech::Tech_log (int x, QString S3)
{
    if(S3==""&&x!=0){
        QString str;
        QDate date;
        QString str1;
        time.start () ;
        str1 = time.toString("_hh_mm_ss");
        date=QDate::currentDate ();
        str = date.toString("dd_MM_yyyy");
        name = "";
        name += "logfile_";
        name += str;
        name += str1;
        name += ".csv";
    }
    else{
        name = "";
        name += S3;
        name += ".csv";
    }
    if(x==1){
        flag_log=1;
        QFile fileOut(name);
        QTextStream writeStream(&fileOut);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        writeStream <<"RM44, DEC:"<< ";"<<"RM44, deg:"<< ";"<<"RM44, min:"<< ";"<<"RM44, sec:"<<";"<<
                      "SKVT V, DEC:"<< ";"<<"SKVT V, deg:"<< ";"<<"SKVT V, min:"<< ";"<<"SKVT V, sec:"<<";"<<
                      "SKVT H, DEC:"<< ";"<<"SKVT H, deg:"<< ";"<<"SKVT H, min:"<< ";"<<"SKVT H, sec:"<<";"<<"Time:"<<"\n";
        fileOut.close();
    }
    else if(x==2){
        flag_log=2;
        QFile fileOut(name);
        QTextStream writeStream(&fileOut);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        writeStream <<"RM44, DEC:"<< ";"<<"RM44, deg:"<< ";"<<"RM44, min:"<< ";"<<"RM44, sec:"<<";"<<
                      "RM44_2, DEC:"<< ";"<<"RM44_2, deg:"<< ";"<<"RM44_2, min:"<< ";"<<"RM44_2, sec:"<<";"<<"Time:"<<"\n";
        fileOut.close();
    }
    if(x==3){
        flag_log=3;
        QFile fileOut(name);
        QTextStream writeStream(&fileOut);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        writeStream <<"RLM, DEC:"<< ";"<<"RLM, deg:"<< ";"<<"RLM, min:"<< ";"<<"RLM, sec:"<<";"<<
                      "SKVT V, DEC:"<< ";"<<"SKVT V, deg:"<< ";"<<"SKVT V, min:"<< ";"<<"SKVT V, sec:"<<";"<<
                      "SKVT H, DEC:"<< ";"<<"SKVT H, deg:"<< ";"<<"SKVT H, min:"<< ";"<<"SKVT H, sec:"<<";"<<"Time:"<<"\n";
        fileOut.close();
    }
    if(x==4){
        flag_log=3;
        QFile fileOut(name);
        QTextStream writeStream(&fileOut);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        writeStream <<"RM, DEC:"<< ";"<<"RM, deg:"<< ";"<<"RM, min:"<< ";"<<"RM, sec:"<<";"<<
                      "RLM, DEC:"<< ";"<<"RLM, deg:"<< ";"<<"RLM, min:"<< ";"<<"RLM, sec:"<<";"<<"Time:"<<"\n";
        fileOut.close();
    }
    if(x==5){
        flag_log=3;
        QFile fileOut(name);
        QTextStream writeStream(&fileOut);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        writeStream <<"RLM, DEC:"<< ";"<<"RLM, deg:"<< ";"<<"RLM, min:"<< ";"<<"RLM, sec:"<<";"<<
                      "RLM, DEC:"<< ";"<<"RLM, deg:"<< ";"<<"RLM, min:"<< ";"<<"RLM, sec:"<<";"<<"Time:"<<"\n";
        fileOut.close();
    }
    else if(x==0)flag_log=0;
}
void Tech::Tech_avt(int x, int y)
{
    if(x==1){
        MyTimerTech = new QTimer;
        MyTimerTech->start(y);
        QObject::connect(MyTimerTech,SIGNAL(timeout()), this, SLOT( Tech_timer()));
    }
    else if (x==0)
    {
        //flag_SSI=0;
        y=0;
        QObject::disconnect(MyTimerTech,SIGNAL(timeout()), this, SLOT( Tech_timer()));
        delete MyTimerTech;
    }
    else if (x==2)
    {
        Tech_timer();
    }
}
void Tech::RLM_RLM()
{
    QByteArray bb;
    bb.resize(4);
    bb[0] = 0x56;
    bb[1] = 0x3e;
    bb[2] = 0x00;
    bb[3] = 0x00;
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x66;
    ba[1] = 0x3e;
    ba[2] = 0x00;
    ba[3] = 0x00;

    com8->QIODevice::write(bb);
    com2->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    com2->waitForBytesWritten(-1);

    QByteArray readData3 = com8->readAll();
    QByteArray readData4 = com9->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    while (com2->waitForReadyRead(10))
        readData4.append(com2->readAll());


    bb[0] = 0x56;
    bb[1] = 0x3f;
    bb[2] = 0x00;
    bb[3] = 0x00;

    ba[0] = 0x66;
    ba[1] = 0x3f;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(bb);
    com2->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    com2->waitForBytesWritten(-1);

    readData3 = com8->readAll();
    readData4 = com9->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    while (com2->waitForReadyRead(10))
        readData4.append(com2->readAll());
    uint8_t uu1,uu2;
    uint8_t uv1,uv2;
    uu1=readData3[2];
    uu2=readData3[3];
    uv1=readData4[2];
    uv2=readData4[3];
    uint32_t pole1,pole2;
    pole1=uu1;
    pole2=uv1;
    if(uu2==4||uv2==4)emit tech_err(800);
    if(uu2==2||uv2==2)emit tech_err(820);
    if(uu2==0||uv2==0)emit tech_err(840);
    if(uu2==0xff||uv2==0xff)emit tech_err(860);

    bb[0] = 0x56;
    bb[1] = 0x40;
    bb[2] = 0x00;
    bb[3] = 0x00;

    ba[0] = 0x66;
    ba[1] = 0x40;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com8->QIODevice::write(bb);
    com2->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    com2->waitForBytesWritten(-1);

    readData3 = com8->readAll();
    readData4 = com9->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    while (com2->waitForReadyRead(10))
        readData4.append(com2->readAll());
    uu1=readData3[2];
    uu2=readData3[3];
    uv1=readData4[2];
    uv2=readData4[3];
    rm1=(pole1<<13)|(uu1<<8) | uu2;
    rm2=(pole2<<13)|(uv1<<8) | uv2;

}
void Tech::RM_RLM()
{
    QByteArray bb;
    bb.resize(4);
    bb[0] = 0x56;
    bb[1] = 0x3d;
    bb[2] = 0x00;
    bb[3] = 0x00;
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0x66;
    ba[1] = 0x3e;
    ba[2] = 0x00;
    ba[3] = 0x00;

    com8->QIODevice::write(bb);
    com2->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    com2->waitForBytesWritten(-1);

    QByteArray readData3 = com8->readAll();
    QByteArray readData4 = com2->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    while (com2->waitForReadyRead(10))
        readData4.append(com2->readAll());
    uint8_t uu1,uu2;
    uu1=readData3[2];
    uu2=readData3[3];
    rm1=(uu1<<8) | uu2;
    ba[0] = 0x66;
    ba[1] = 0x3f;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com2->QIODevice::write(ba);
    readData3 = com2->readAll();
    com2->waitForBytesWritten(-1);
    while (com2->waitForReadyRead(10))
        readData3.append(com2->readAll());
    uu1=readData3[2];
    uu2=readData3[3];
    uint32_t pole;
    pole=uu1;
    if(uu2==4)emit tech_err(800);
    if(uu2==2)emit tech_err(820);
    if(uu2==0)emit tech_err(840);
    if(uu2==0xff)emit tech_err(860);

    ba[0] = 0x66;
    ba[1] = 0x40;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com2->QIODevice::write(ba);
    readData3 = com2->readAll();
    com2->waitForBytesWritten(-1);
    while (com2->waitForReadyRead(10))
        readData3.append(com2->readAll());
    uu1=readData3[2];
    uu2=readData3[3];
    rm2=(pole<<13)|(uu1<<8) | uu2;

}
void Tech::SKVT_RLM()
{
    QByteArray bb;
    bb.resize(4);
    bb[0] = 0x56;
    bb[1] = 0x3e;
    bb[2] = 0x00;
    bb[3] = 0x00;
    /*QByteArray ba;
    ba.resize(6);
    ba[0] = 0xAB;
    ba[1] = 0xCD;
    ba[2] = 0x00;
    ba[3] = 0x20;
    ba[4] = 0x0A;
    ba[5] = 0x0D;*/

    com8->QIODevice::write(bb);
    //com9->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    //com9->waitForBytesWritten(-1);

    QByteArray readData3 = com8->readAll();
    QByteArray readData4 = com9->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    /*while (com9->waitForReadyRead(10))
        readData4.append(com9->readAll());*/
    bb[0] = 0x56;
    bb[1] = 0x3f;
    bb[2] = 0x00;
    bb[3] = 0x00;
    com8->QIODevice::write(bb);
    com8->waitForBytesWritten(-1);
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    uint8_t uu1,uu2;
    uu1=readData3[2];
    uu2=readData3[3];
    uint32_t pole;
    pole=uu1;
    if(uu2==4)emit tech_err(800);
    if(uu2==2)emit tech_err(820);
    if(uu2==0)emit tech_err(840);
    if(uu2==0xff)emit tech_err(860);

    bb[0] = 0x56;
    bb[1] = 0x40;
    bb[2] = 0x00;
    bb[3] = 0x00;
    com8->QIODevice::write(bb);
    com8->waitForBytesWritten(-1);
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    uu1=readData3[2];
    uu2=readData3[3];
    rm1=(pole<<13)|(uu1<<8) | uu2;
    uint8_t v1,v2,h1,h2;
    v1=readData4[10];
    v2=readData4[11];
    skvt_V=(v1<<8) | v2;
    h1=readData4[12];
    h2=readData4[13];
    skvt_H=(h1<<8) | h2;
}
void SKVT_angle()
{
    QByteArray bb;
    bb.resize(4);
    bb[0] = 0x56;
    bb[1] = 0x3d;
    bb[2] = 0x00;
    bb[3] = 0x00;
    QByteArray ba;
    ba.resize(6);
    ba[0] = 0xAB;
    ba[1] = 0xCD;
    ba[2] = 0x00;
    ba[3] = 0x20;
    ba[4] = 0x0A;
    ba[5] = 0x0D;

    com8->QIODevice::write(bb);
    com9->QIODevice::write(ba);
    com8->waitForBytesWritten(-1);
    com9->waitForBytesWritten(-1);

    QByteArray readData3 = com8->readAll();
    QByteArray readData4 = com9->readAll();
    while (com8->waitForReadyRead(10))
        readData3.append(com8->readAll());
    while (com9->waitForReadyRead(10))
        readData4.append(com9->readAll());
    uint8_t uu1,uu2;
    uu1=readData3[2];
    uu2=readData3[3];
    rm1=(uu1<<8) | uu2;
    uint8_t v1,v2,h1,h2;
    v1=readData4[10];
    v2=readData4[11];
    skvt_V=(v1<<8) | v2;
    h1=readData4[12];
    h2=readData4[13];
    skvt_H=(h1<<8) | h2;
}
void Tech::Tech_timer()
{
    double x=0,y=0;
    QString str;
    time.start () ;
    if(flag_SSI==1) {
        SKVT_angle();
        x=rm1;
    }
    if(flag_SSI==2) {
        tech_read_RM_2();
        x=rm1;
        y=rm2;
        if (y>4095){
            y=y-4096;
        }
        //y=y*16;
    }
    x=8191-x;
    if (x>4095){
        x=x-4096;
    }
    //x=x*16;
    if(flag_SSI==3) {
        SKVT_RLM();
        y=1581055-rm1;
        if (y>790527){
            y=y-790528;
        }
        /*uint16_t s1,s2;
        s1=skvt_V;
        s2=skvt_H;
        skvt_V=(s1<<16) | s2;*/
        x=y;
    }
    if(flag_SSI==4) {
        RM_RLM();
        x=8191-rm1;
        y=rm2;
    }
    if(flag_SSI==5) {
        RLM_RLM();
        x=1581055-rm1;
        y=rm2;
    }
    int a1=1,a2=1;
    if(x==1||x==2){
        a2=1;
        a1=1;
    }
    if(x==3||x==5){
        a2=192;
        a1=192;
    }
    if(x==4){
        a2=192;
        a1=1;
    }
    str = time.toString("hh:mm:ss.zzz");
    int degree = 180*x/(8192*a1);
    double z1=(180*x/(8192.0*a1)-degree)*60.0;
    int minute =z1;
    int sec = (z1-minute)*60.0;
    int degree_2 = 180*y/(8192*a2);
    double z2=(180*y/(8192.0*a2)-degree_2)*60.0;
    int minute_2 =z2;
    int sec_2 = (z2-minute_2)*60.0;
    int degreev = 180*skvt_V/65536;
    double zv=(180*skvt_V/65536.0-degreev)*60.0;
    int minutev =zv;
    int secv = (zv-minutev)*60.0;
    int degreeh = 180*skvt_H/65536;
    double zh=(180*skvt_H/65536.0-degreeh)*60.0;
    int minuteh =zh;
    int sech = (zh-minuteh)*60.0;

    if(flag_log==1){
        QFile fileOut(name);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        QTextStream writeStream(&fileOut);
        writeStream <<x<< ";"<<degree<< ";"<<minute<< ";"<<sec<<";"<<
                      skvt_V<< ";"<<degreev<< ";"<<minutev<< ";"<<secv<<";"<<
                      skvt_H<< ";"<<degreeh<< ";"<<minuteh<< ";"<<sech<<";"<<str<<"\n";
        fileOut.close();
    }
    if(flag_log==2||flag_log==4||flag_log==5){
        QFile fileOut(name);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        QTextStream writeStream(&fileOut);
        writeStream <<x<< ";"<<degree<< ";"<<minute<< ";"<<sec<<";"<<
                      y<< ";"<<degree_2<< ";"<<minute_2<< ";"<<sec_2<<";"<<str<<"\n";
        fileOut.close();
    }
    if(flag_log==3){
        QFile fileOut(name);
        fileOut.open(QIODevice::Append | QIODevice::Text);
        QTextStream writeStream(&fileOut);
        writeStream <<x<< ";"<<degree<< ";"<<minute<< ";"<<sec<<";"<<
                      skvt_V<< ";"<<degreev<< ";"<<minutev<< ";"<<secv<<";"<<
                      skvt_H<< ";"<<degreeh<< ";"<<minuteh<< ";"<<sech<<";"<<str<<"\n";
        fileOut.close();
    }
   emit tech_SSI_value( x, y, skvt_V, skvt_H);
}

void Tech:: COMCloseTech(){
    com8->close();
    com9->close();
}

