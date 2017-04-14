#include "comport.h"

#include <QObject>
#include <QString>
#include "qapplication.h"
#include <QtSerialPort/QtSerialPort>
#include "synchapi.h"
int flag_rem;
QSerialPort *com6,*com5,*com4;
QByteArray ba;
double ii1,ii2;
uint8_t er1,er2;
int flag_res_stm=1;
void COMClose5_6(){

    com5->close();
    com6->close();
}

void COMClose4(){
    com4->close();
}
void startpower()
{
    Remote_ON();
    ba.resize(7);
    ba[0] = 0xf1;//power off
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x28;
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData1 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData1.append(com5->readAll());
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData2 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData2.append(com6->readAll());
    setUIcom5(27);
    setUIcom6(27);
    setoverUIcom6 (29, 2);
    setoverUIcom6 (29, 2);


}
void COMConnector5_6()
{
    com5 = new QSerialPort("com5");
    if (com5->open(QIODevice::ReadWrite))
    {
        com5->setBaudRate(QSerialPort::Baud115200);
        com5->setDataBits(QSerialPort::Data8);
        com5->setParity(QSerialPort::OddParity);
        com5->setStopBits(QSerialPort::OneStop);
        com5->setFlowControl(QSerialPort::NoFlowControl);
    }

    com6 = new QSerialPort("com6");
    com6->open(QIODevice::ReadWrite);
    com6->setBaudRate(QSerialPort::Baud115200);
    com6->setDataBits(QSerialPort::Data8);
    com6->setParity(QSerialPort::OddParity);
    com6->setStopBits(QSerialPort::OneStop);
    com6->setFlowControl(QSerialPort::NoFlowControl);

    startpower();
}

void COMConnector4()
{
    com4 = new QSerialPort("com4");
    com4->open(QIODevice::ReadWrite);
    com4->setBaudRate(QSerialPort::Baud115200);
    com4->setDataBits(QSerialPort::Data8);
    com4->setParity(QSerialPort::OddParity);
    com4->setStopBits(QSerialPort::OneStop);
    com4->setFlowControl(QSerialPort::NoFlowControl);
}
int stm_on_com6(int y,int x)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0b;
    ba[2] = y;
    ba[3] = x;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    return readData1[3];
}
int echo_com()
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x07;
    ba[2] = 0xaa;
    ba[3] = 0xaa;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    if(readData1[3]==0xaa && readData1[2]==0xaa)
        return 1;
    else return 0;
}
int stm_on_mko(int x, int y)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0e;
    ba[2] = x;
    ba[3] = y;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    Sleep(100);
    return readData1[3];
}


int ctm_check_fuse(int fuse)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0c;
    ba[2] = fuse;
    ba[3] = 0x00;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    return readData1[3];
}
int stm_on_com5(int y, int x)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0b;
    ba[2] = y;
    ba[3] = x;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    return readData1[3];
}
double ctm_data_ch(int ch)
{
    if(flag_res_stm==1) {
        ba.resize(4);
        ba[0] = 0x22;
        ba[1] = 0x0d;
        ba[2] = ch;
        ba[3] = 0x00;
        com4->QIODevice::write(ba);
        com4->waitForBytesWritten(-1);
        QByteArray readData1 = com4->readAll();
        while (com4->waitForReadyRead(100))
            readData1.append(com4->readAll());
        uint8_t uu1,uu2;
        uu1=readData1[2];
        uu2=readData1[3];
        double res=(uu1<<8) | uu2;
        return res;
    }
    return 50000;
}
void Reset_error_com6()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x0a;
    ba[4] = 0x0a;
    ba[5] = 0x01;
    ba[6] = 0x3b;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());
}
void Reset_error_com5()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x0a;
    ba[4] = 0x0a;
    ba[5] = 0x01;
    ba[6] = 0x3b;
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData2 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData2.append(com5->readAll());
}
void Remote_ON()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x10;
    ba[4] = 0x10;
    ba[5] = 0x01;
    ba[6] = 0x47;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData2 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData2.append(com5->readAll());
}
void Remote_OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x10;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x37;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData2 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData2.append(com5->readAll());
}
void com6ON()
{
    ba.resize(7);
    ba[0] = 0xf1;//power on
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x01;
    ba[5] = 0x01;
    ba[6] = 0x29;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());
}

void com6OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;//power off
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x28;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());
}
void com5ON()
{
    ba.resize(7);
    ba[0] = 0xf1;//power on
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x01;
    ba[5] = 0x01;
    ba[6] = 0x29;
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData2 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData2.append(com5->readAll());
}

void com5OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;//power off
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x28;
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData2 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData2.append(com5->readAll());
}
int readcom5U()
{
    ba.resize(5);
    ba[0] = 0x75;
    ba[1] = 0x00;
    ba[2] = 0x47;
    ba[3] = 0x00;
    ba[4] = 0xbc;
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData.append(com5->readAll());
    uint8_t uu1,uu2;
    er2=(readData[4]>>4);
    uu1=readData[5];
    uu2=readData[6];
    double uu=(uu1<<8) | uu2;
    uu=uu*42/(256);
    uu1=readData[7];
    uu2=readData[8];
    ii2=(uu1<<8) | uu2;
    ii2=ii2*10/(256);
    return uu;
}
int readcom5I()
{
    return ii2;
}
int readerr4I()
{
    return er2;
}
int readerr11I()
{
    return er1;
}
int readcom6U()
{
    ba.resize(5);
    ba[0] = 0x75;
    ba[1] = 0x00;
    ba[2] = 0x47;
    ba[3] = 0x00;
    ba[4] = 0xbc;
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData.append(com6->readAll());
    uint8_t uu1,uu2;
    er1=(readData[4]>>4);
    uu1=readData[5];
    uu2=readData[6];
    double uu=(uu1<<8) | uu2;
    uu=uu*42/(256);
    uu1=readData[7];
    uu2=readData[8];
    ii1=(uu1<<8) | uu2;
    ii1=ii1*10/(256);
    return uu;
}
int readcom6I()
{
    return ii1;
}
void setUIcom5(double u)
{
    ba.resize(7);
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x32;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData.append(com5->readAll());

    uint32_t vali = (155*25600/u)/10;//I
    if(vali>25600) vali=25600;
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x33;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData1 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData1.append(com5->readAll());
}

void setUIcom6(double u)
{
    uint16_t sum=0;
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x32;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;

    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());

    uint32_t vali = (155*25600/u)/10;//I
    //uint32_t vali = (0.075*25600)/10;//I
    if(vali>25600) vali=25600;
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x33;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData2 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData2.append(com6->readAll());
    ba.resize(7);

}

void setoverUIcom5(double u,double ii)
{
    ba.resize(7);
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x26;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData.append(com5->readAll());

    uint32_t vali = (ii*25600)/10;//I
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x27;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com5->QIODevice::write(ba);
    com5->waitForBytesWritten(-1);
    QByteArray readData1 = com5->readAll();
    while (com5->waitForReadyRead(100))
        readData1.append(com5->readAll());
}

void setoverUIcom6(double u,double ii)
{
    ba.resize(7);
    uint32_t valU = ((u+0.1)*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x26;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData1 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData1.append(com6->readAll());

    uint32_t vali = (ii*25600)/10;//I
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x27;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0; i<5; i++) {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    com6->QIODevice::write(ba);
    com6->waitForBytesWritten(-1);
    QByteArray readData2 = com6->readAll();
    while (com6->waitForReadyRead(100))
        readData2.append(com6->readAll());
}
int id_stm()
{
    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x01;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());

    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x02;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData2 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData2.append(com4->readAll());
    if(readData1[2]==readData2[2]&&readData1[3]==readData2[3]) return 1;
    else return 0;
}



QString req_stm()
{
    if(flag_res_stm==1) {
        QString res;
        ba.resize(4);
        ba[0] = 0x22;
        ba[1] = 0x02;
        ba[2] = 0x00;
        ba[3] = 0x00;
        com4->QIODevice::write(ba);
        com4->waitForBytesWritten(-1);
        QByteArray readData1 = com4->readAll();
        while (com4->waitForReadyRead(100))
            readData1.append(com4->readAll());
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        res="";
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1==0) res+="СТМ: Модуль не готов к работе! \n";
        if(z2==1) res+="СТМ: Ошибки у модуля! \n";
        if(z3==1) res+="СТМ: Модуль после перезагрузки! \n";
        return res;
    }
    return "";
}
int res_err_stm()
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x03;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    return readData1[3];
}

int res_stm()
{
    flag_res_stm=0;
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x04;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    COMClose4 ();
    for(int i=0; i<500; i++) {
        Sleep(10);
        QApplication::processEvents();
    }
    COMConnector4();
    flag_res_stm=1;
    return readData1[3];
}
int fw_stm()
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x06;
    ba[2] = 0x00;
    ba[3] = 0x00;
    com4->QIODevice::write(ba);
    com4->waitForBytesWritten(-1);
    QByteArray readData1 = com4->readAll();
    while (com4->waitForReadyRead(100))
        readData1.append(com4->readAll());
    return (readData1[2]*10+readData1[3]);
}






/*QSerialPort *com7;
   QByteArray bw;
   QString data,temp;
   QByteArray readData0;
   int len1=0, len2=0;
   void COMConnectorOTD()
   {
    com7 = new QSerialPort("com7");
    com7->open(QIODevice::ReadWrite);
    com7->setBaudRate(QSerialPort::Baud115200);
    com7->setDataBits(QSerialPort::Data5);
    com7->setParity(QSerialPort::OddParity);
    com7->setStopBits(QSerialPort::OneStop);
    com7->setFlowControl(QSerialPort::NoFlowControl);
    //OTDtemper();

   }

   void OTDres1()
   {
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x26;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    //if(readData1[3]==2)emit err_OTD("Ошибка при перезагрузке!");
   }
   void OTDres2()
   {
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x27;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    //if(readData1[3]==2)emit err_OTD("Ошибка при перезагрузке!");
   }
   void OTDmeas1()
   {
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x28;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    //if(readData1[3]==2)emit err_OTD("Ошибка при запуске измерений 1-й линии!");
   }
   void OTDmeas2()
   {
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x29;
    bw[2] = 0x00;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    //if(readData1[3]==2)emit err_OTD("Ошибка при запуске измерений 2-й линии!");
   }
   QString OTDtemper()
   {
   // Sleep(1000);
    data="";
    data+="Кол-во датчиков DS1820 по оси 1: \n";
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x1e;
    bw[2] = 0x00;
    bw[3] = 0x00;
    //com7->QIODevice::write(bw);
    //com7->waitForBytesWritten(-1);
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    readData0 = com7->readAll();
    while (com7->waitForReadyRead(500))
        readData0.append(com7->readAll());
    len1=readData0[2];
    data+=QString::number(len1);
    data+="\n";
    data+="Адреса датчиков DS1820 по оси 1: \n";
    for(int j=1;j<=len1;j++)
    {
        data+=QString::number(j);
        data+=" : ";
        for(int k=0;k<8;k++)
        {
            bw[0] = 0x44;
            bw[1] = 0x2a;
            bw[2] = 0x00;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData3 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData3.append(com7->readAll());
            data+=QString::number(readData3[2]);
        }
        data+= "\n";
    }
    data+="\n\n";
    if(readData0[3]==2)data+="Ошибка при считывании датчиков 1-й оси\n";

    data+="Кол-во датчиков DS1820 по оси 2: \n";
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x1d;
    bw[2] = 0x00;
    bw[3] = 0x00;
    //com7->QIODevice::write(bw);
    //com7->waitForBytesWritten(-1);
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData3 = com7->readAll();
    while (com7->waitForReadyRead(500))
        readData3.append(com7->readAll());
    len2=readData3[2];
    data+=QString::number(len2);
    data+="\n";
    data+="Адреса датчиков DS1820 по оси 2: \n";
    for(int j=1;j<=len2;j++)
    {
        data+=QString::number(j);
        data+=" : ";
        for(int k=0;k<8;k++)
        {
            bw[0] = 0x44;
            bw[1] = 0x2a;
            bw[2] = 0x00;
            bw[3] = 0x00;
            com7->QIODevice::write(bw);
            com7->waitForBytesWritten(-1);
            QByteArray readData4 = com7->readAll();
            while (com7->waitForReadyRead(500))
                readData4.append(com7->readAll());
            data+=QString::number(readData4[2]);
        }
        data+= "\n";
    }
    if(readData3[3]==2)data+="Ошибка при считывании датчиков 2-й оси\n";
    return data;
     //emit temp_OTD(data);

   }
   void OTDtm1()
   {
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
        while (com7->waitForReadyRead(100))
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
        temp+=QString::number(uu);
        temp+="\n";
    }
    //emit tm_OTD(temp);
   }
   void OTDtm2()
   {
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
        while (com7->waitForReadyRead(100))
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
        temp+=QString::number(uu);
        temp+="\n";
    }
    //emit tm_OTD(temp);
   }
   int OTDPT()
   {
    bw.resize(4);
    bw[0] = 0x44;
    bw[1] = 0x1c;
    bw[2] = 0x01;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData1 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData1.append(com7->readAll());
    uint8_t uu1,uu2;
    uu1=readData1[2];
    uu2=readData1[3];
    double uu=(uu1<<8) | uu2;
    uu=uu/32-256;

    bw[0] = 0x44;
    bw[1] = 0x1c;
    bw[2] = 0x02;
    bw[3] = 0x00;
    com7->QIODevice::write(bw);
    com7->waitForBytesWritten(-1);
    QByteArray readData2 = com7->readAll();
    while (com7->waitForReadyRead(100))
        readData2.append(com7->readAll());
    uu1=readData2[2];
    uu2=readData2[3];
    double uu3=(uu1<<8) | uu2;
    uu3=uu3/32-256;
    return uu3*100;
    //emit start_OTDPT(uu, uu3);

   }

   void  COMCloseOTD(){
    com7->close();
   }
 */
