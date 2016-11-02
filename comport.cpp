#include "comport.h"

#include <QObject>
#include <QString>
#include "qapplication.h"
#include "synchapi.h"


COMPortSender::COMPortSender(QObject *parent):
    QObject(parent)
{

}

COMPortSender::~COMPortSender()
{
    if (m_com4 != Q_NULLPTR && m_com4->isOpen())
    {
        m_com4->close();
    }

    if (m_com5 != Q_NULLPTR && m_com5->isOpen())
    {
        m_com5->close();
    }

    if (m_com6 != Q_NULLPTR && m_com6->isOpen())
    {
        m_com6->close();
    }

    if (m_com8 != Q_NULLPTR && m_com8->isOpen())
    {
        m_com8->close();
    }
}

void COMPortSender::createPorts()
{
    m_com4 = createPort("com4");
    m_com5 = createPort("com5");
    m_com6 = createPort("com6");
    m_com8 = createPort("com8");
}

QSerialPort * COMPortSender::createPort(const QString& name)
{
    QSerialPort *port = new QSerialPort(name, this);
    port->open(QIODevice::ReadWrite);
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data5);
    port->setParity(QSerialPort::OddParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    return port;

    //m_ports.push_back(port);
}

void COMPortSender::startPower()
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

    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData1 = m_com5->readAll();

    while (m_com5->waitForReadyRead(100))
    {
        readData1.append(m_com5->readAll());
    }

    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData2 = m_com6->readAll();

    while (m_com6->waitForReadyRead(100))
    {
        readData2.append(m_com6->readAll());
    }

    setUIcom5(0.5);
    setUIcom6(0.5);
}

int COMPortSender::stm_on_com6(int y,int x)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0b;
    ba[2] = y;
    ba[3] = x;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }

    return readData1[3];
}
int COMPortSender::stm_on_mko(int x, int y)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0e;
    ba[2] = x;
    ba[3] = y;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }
    return readData1[3];
}

int COMPortSender::ctm_check_fuse(int fuse)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0c;
    ba[2] = fuse;
    ba[3] = 0x00;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }

    return readData1[3];
}

int COMPortSender::stm_on_com5(int y, int x)
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x0b;
    ba[2] = y;
    ba[3] = x;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }

    return readData1[3];
}

int COMPortSender::tech_send(int com, int x, int y)
{
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = com;
    ba[2] = x;
    ba[3] = y;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData2 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData2.append(m_com8->readAll());
    }

    return readData2[3];
}

int COMPortSender::tech_read(int x)
{
    if(flag_res_tech == 1)
    {
        int y = 0;
        if(x == 1)
        {
            y = 25;
        }
        else
        {
            y = 19;
        }

        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = y;
        ba[2] = 0x00;
        ba[3] = 0x00;
        m_com8->QIODevice::write(ba);
        m_com8->waitForBytesWritten(-1);
        QByteArray readData2 = m_com8->readAll();
        while (m_com8->waitForReadyRead(100))
        {
            readData2.append(m_com8->readAll());
        }

        uint8_t uu1,uu2;
        uu1=readData2[2];
        uu2=readData2[3];
        double uu=(uu1<<8) | uu2;
        return uu;
    }

    return 0;
}

QString COMPortSender::tech_read_buf(int x,int len)
{
    int y = 0;
    if(x==1)
    {
        y=26;
    }
    else
    {
        y=20;
    }

    QString result;
    for(int i = 0; i < len; i++)
    {
        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = y;
        ba[2] = 0x00;
        ba[3] = 0x00;
        m_com8->QIODevice::write(ba);
        m_com8->waitForBytesWritten(-1);
        QByteArray readData2 = m_com8->readAll();
        while (m_com8->waitForReadyRead(100))
        {
            readData2.append(m_com8->readAll());
        }

        if(readData2.at(2) == 1)
        {
            result+="em ";
        }

        if(readData2.at(2) == 2)
        {
            result+="uu ";
        }

        result+=readData2[3];
        result+=" ";
        QApplication::processEvents();
    }

    return result;
}

double COMPortSender::ctm_data_ch(int ch)
{
    if (flag_res_stm==1)
    {
        ba.resize(4);
        ba[0] = 0x22;
        ba[1] = 0x0d;
        ba[2] = ch;
        ba[3] = 0x00;
        m_com4->QIODevice::write(ba);
        m_com4->waitForBytesWritten(-1);
        QByteArray readData1 = m_com4->readAll();
        while (m_com4->waitForReadyRead(100))
        {
            readData1.append(m_com4->readAll());
        }

        uint8_t uu1,uu2;
        uu1=readData1[2];
        uu2=readData1[3];
        double res=(uu1<<8) | uu2;
        return res;
    }

    return 50000;
}

void COMPortSender::Reset_error_com6()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x0a;
    ba[4] = 0x0a;
    ba[5] = 0x01;
    ba[6] = 0x3b;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
        readData1.append(m_com6->readAll());
}

void COMPortSender::Reset_error_com5()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x0a;
    ba[4] = 0x0a;
    ba[5] = 0x01;
    ba[6] = 0x3b;
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData2 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData2.append(m_com5->readAll());
    }
}

void COMPortSender::Remote_ON()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x10;
    ba[4] = 0x10;
    ba[5] = 0x01;
    ba[6] = 0x47;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }

    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData2 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData2.append(m_com5->readAll());
    }
}

void COMPortSender::Remote_OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x10;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x37;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }

    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData2 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData2.append(m_com5->readAll());
    }
}
void COMPortSender::com6ON()
{
    ba.resize(7);
    ba[0] = 0xf1;//power on
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x01;
    ba[5] = 0x01;
    ba[6] = 0x29;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }
}

void COMPortSender::com6OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;//power off
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x28;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }
}
void COMPortSender::com5ON()
{
    ba.resize(7);
    ba[0] = 0xf1;//power on
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x01;
    ba[5] = 0x01;
    ba[6] = 0x29;
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData2 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData2.append(m_com5->readAll());
    }
}

void COMPortSender::com5OFF()
{
    ba.resize(7);
    ba[0] = 0xf1;//power off
    ba[1] = 0x00;
    ba[2] = 0x36;
    ba[3] = 0x01;
    ba[4] = 0x00;
    ba[5] = 0x01;
    ba[6] = 0x28;
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData2 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData2.append(m_com5->readAll());
    }
}
int COMPortSender::readcom5U()
{
    ba.resize(5);
    ba[0] = 0x75;
    ba[1] = 0x00;
    ba[2] = 0x47;
    ba[3] = 0x00;
    ba[4] = 0xbc;
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData.append(m_com5->readAll());
    }

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

int COMPortSender::readcom5I()
{
    return ii1;
}

int COMPortSender::readerr4I()
{
    return er2;
}

int COMPortSender::readerr11I()
{
    return er1;
}

int COMPortSender::readcom6U()
{
    ba.resize(5);
    ba[0] = 0x75;
    ba[1] = 0x00;
    ba[2] = 0x47;
    ba[3] = 0x00;
    ba[4] = 0xbc;
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData.append(m_com6->readAll());
    }

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

int COMPortSender::readcom6I()
{
    return ii1;
}

void COMPortSender::setUIcom5(double u)
{
    ba.resize(7);
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x32;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }

    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData.append(m_com5->readAll());
    }

    uint32_t vali = (155*25600/u)/10;//I
    if(vali>25600)vali=25600;
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x33;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }

    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData1 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData1.append(m_com5->readAll());
    }
}

void COMPortSender::setUIcom6(double u)
{
    uint16_t sum=0;
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x32;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;

    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }

    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }

    uint32_t vali = (155*25600/u)/10;//I
    if(vali>25600)vali=25600;
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x33;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData2 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData2.append(m_com6->readAll());
    }

    ba.resize(7);
}

void COMPortSender::setoverUIcom5(double u,double ii)
{
    ba.resize(7);
    uint32_t valU = (u*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x26;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData.append(m_com5->readAll());
    }

    uint32_t vali = (ii*25600)/10;//I
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x27;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com5->QIODevice::write(ba);
    m_com5->waitForBytesWritten(-1);
    QByteArray readData1 = m_com5->readAll();
    while (m_com5->waitForReadyRead(100))
    {
        readData1.append(m_com5->readAll());
    }
}

void COMPortSender::setoverUIcom6(double u,double ii)
{
    ba.resize(7);
    uint32_t valU = ((u+0.1)*25600)/42;//U
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x26;
    ba[3] = ( valU >> 8 ) & 0xFF;
    ba[4] = valU & 0xFF;
    uint16_t sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData1 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData1.append(m_com6->readAll());
    }

    uint32_t vali = (ii*25600)/10;//I
    ba[0] = 0xf1;
    ba[1] = 0x00;
    ba[2] = 0x27;
    ba[3] = ( vali >> 8 ) & 0xFF;
    ba[4] = vali & 0xFF;
    sum=0;
    for(int i=0;i<5;i++)
    {
        uint8_t s=ba[i];
        sum=(sum+s)& 0xFFFF;
    }
    ba[5] =(( sum >> 8 ) & 0xFF);
    ba[6] =  (sum  & 0xFF);
    m_com6->QIODevice::write(ba);
    m_com6->waitForBytesWritten(-1);
    QByteArray readData2 = m_com6->readAll();
    while (m_com6->waitForReadyRead(100))
    {
        readData2.append(m_com6->readAll());
    }
}
int COMPortSender::id_stm()
{
    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x01;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }

    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x02;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData2 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData2.append(m_com4->readAll());
    }

    if(readData1[2]==readData2[2]&&readData1[3]==readData2[3])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


int COMPortSender::id_tech()
{
    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x01;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData1 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData1.append(m_com8->readAll());
    }

    ba.resize(4);
    ba[0] = 0xff;
    ba[1] = 0x01;
    ba[2] = 0x00;
    ba[3] = 0x02;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData2 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData2.append(m_com8->readAll());
    }

    if(readData1[2]==readData2[2]&&readData1[3]==readData2[3])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
QString COMPortSender::req_stm()
{
    if(flag_res_stm==1)
    {
        QString res;
        ba.resize(4);
        ba[0] = 0x22;
        ba[1] = 0x02;
        ba[2] = 0x00;
        ba[3] = 0x00;
        m_com4->QIODevice::write(ba);
        m_com4->waitForBytesWritten(-1);
        QByteArray readData1 = m_com4->readAll();
        while (m_com4->waitForReadyRead(100))
            readData1.append(m_com4->readAll());
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        res="";
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1==0)
            res+=" СТМ не готов к работе! \n";
        if(z2==1)
            res+=" Ошибки у модуля СТМ! \n";
        if(z3==1)
            res+=" Модуль СТМ после перезагрузки! \n";
        return res;
    }

    return "";
}

QString COMPortSender::req_tech()
{
    if(flag_res_tech==1)
    {
        QString res;
        ba.resize(4);
        ba[0] = 0x56;
        ba[1] = 0x02;
        ba[2] = 0x00;
        ba[3] = 0x00;
        m_com8->QIODevice::write(ba);
        m_com8->waitForBytesWritten(-1);
        QByteArray readData1 = m_com8->readAll();
        while (m_com8->waitForReadyRead(100))
        {
            readData1.append(m_com8->readAll());
        }
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1==0)
            res+=" Технол. модуль не готов к работе! \n";
        if(z2==1)
            res+=" Ошибки у Технол. модуля! \n";
        if(z3==1)
            res+=" Модуль Технол. после перезагрузки! \n";
        if(readData1.at(3)==0x10)
            res+=" Потеря байта из-за переполнения буфера RS485! \n";
        return res;
    }
    return "";
}
int COMPortSender::res_err_stm()
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x03;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }
    return readData1[3];
}

int COMPortSender::res_err_tech()
{
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x03;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData1 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData1.append(m_com8->readAll());
    }
    return readData1[3];
}

int COMPortSender::res_stm()
{
    flag_res_stm=0;
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x04;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }

    if (m_com4 != Q_NULLPTR && m_com4->isOpen())
    {
        m_com4->close();
        m_com4->deleteLater();
        m_com4 = Q_NULLPTR;
    }

    for(int i = 0; i < 300; i++)
    {
        Sleep(10);
        QApplication::processEvents();
    }

    m_com4 = createPort("com4");

    flag_res_stm=1;
    return readData1[3];
}

int COMPortSender::res_tech()
{
    flag_res_tech=0;
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x04;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData1 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData1.append(m_com8->readAll());
    }

    if (m_com8 != Q_NULLPTR && m_com8->isOpen())
    {
        m_com8->close();
        m_com8->deleteLater();
        m_com8 = Q_NULLPTR;
    }

    for(int i = 0; i < 400; i++)
    {
        Sleep(10);
        QApplication::processEvents();
    }

    m_com8 = createPort("com8");
    flag_res_tech = 1;
    return readData1[3];
}

int COMPortSender::fw_stm()
{
    ba.resize(4);
    ba[0] = 0x22;
    ba[1] = 0x06;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com4->QIODevice::write(ba);
    m_com4->waitForBytesWritten(-1);
    QByteArray readData1 = m_com4->readAll();
    while (m_com4->waitForReadyRead(100))
    {
        readData1.append(m_com4->readAll());
    }
    return (readData1[2]*10+readData1[3]);
}

int COMPortSender::fw_tech()
{
    ba.resize(4);
    ba[0] = 0x56;
    ba[1] = 0x06;
    ba[2] = 0x00;
    ba[3] = 0x00;
    m_com8->QIODevice::write(ba);
    m_com8->waitForBytesWritten(-1);
    QByteArray readData1 = m_com8->readAll();
    while (m_com8->waitForReadyRead(100))
    {
        readData1.append(m_com8->readAll());
    }
    return (readData1[2]*10+readData1[3]);
}

