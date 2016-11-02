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
    // TODO: The order of ports creation possibly important!
    m_com5 = createPort("com5");
    m_com6 = createPort("com6");
    m_com4 = createPort("com4");
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

QByteArray COMPortSender::send(QSerialPort * port, QByteArray data)
{
    port->QIODevice::write(data);
    port->waitForBytesWritten(-1);

    QByteArray readData = port->readAll();

    while (port->waitForReadyRead(100))
    {
        readData.append(port->readAll());
    }

    return readData;
}

void COMPortSender::startPower()
{
    // PowerON
    QByteArray buffer1(7, 0);
    buffer1[0] = 0xf1;
    buffer1[1] = 0x00;
    buffer1[2] = 0x36;
    buffer1[3] = 0x10;
    buffer1[4] = 0x10;
    buffer1[5] = 0x01;
    buffer1[6] = 0x47;
    QByteArray readData11 = send(m_com6, buffer1);
    QByteArray readData21 = send(m_com5, buffer1);

    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x28;

    QByteArray readData1 = send(m_com5, buffer);
    QByteArray readData2 = send(m_com6, buffer);

    setUIcom5(0.5);
    setUIcom6(0.5);
}

int COMPortSender::stm_on_com6(int y,int x)
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x0b;
    buffer[2] = y;
    buffer[3] = x;

    QByteArray readData1 = send(m_com4, buffer);
    return readData1[3];
}

int COMPortSender::stm_on_mko(int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x0e;
    buffer[2] = x;
    buffer[3] = y;

    QByteArray readData1 = send(m_com4, buffer);
    return readData1[3];
}

int COMPortSender::stm_check_fuse(int fuse)
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x0c;
    buffer[2] = fuse;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com4, buffer);
    return readData1[3];
}

int COMPortSender::stm_on_com5(int y, int x)
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x0b;
    buffer[2] = y;
    buffer[3] = x;
    QByteArray readData1 = send(m_com4, buffer);
    return readData1[3];
}

int COMPortSender::tech_send(int com, int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x56;
    buffer[1] = com;
    buffer[2] = x;
    buffer[3] = y;
    QByteArray readData1 = send(m_com8, buffer);
    return readData1[3];
}

int COMPortSender::tech_read(int x)
{
    if(m_flag_res_tech == 1)
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

        QByteArray buffer(4, 0);
        buffer[0] = 0x56;
        buffer[1] = y;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(m_com8, buffer);

        uint8_t uu1,uu2;
        uu1 = readData2[2];
        uu2 = readData2[3];
        double uu = (uu1 << 8) | uu2;
        return uu;
    }

    return 0;
}

QString COMPortSender::tech_read_buf(int x,int len)
{
    int y = 0;
    if(x == 1)
    {
        y = 26;
    }
    else
    {
        y = 20;
    }

    QString result;
    QByteArray buffer(4, 0);
    for(int i = 0; i < len; i++)
    {
        buffer[0] = 0x56;
        buffer[1] = y;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(m_com8, buffer);

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

double COMPortSender::stm_data_ch(int ch)
{
    if (m_flag_res_stm == 1)
    {
        QByteArray buffer(4, 0);
        buffer[0] = 0x22;
        buffer[1] = 0x0d;
        buffer[2] = ch;
        buffer[3] = 0x00;

        QByteArray readData1 = send(m_com4, buffer);

        uint8_t uu1, uu2;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double res = (uu1 << 8) | uu2;
        return res;
    }

    return 50000;
}

void COMPortSender::Reset_error_com6()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x0a;
    buffer[4] = 0x0a;
    buffer[5] = 0x01;
    buffer[6] = 0x3b;
    QByteArray readData1 = send(m_com6, buffer);
}

void COMPortSender::Reset_error_com5()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x0a;
    buffer[4] = 0x0a;
    buffer[5] = 0x01;
    buffer[6] = 0x3b;
    QByteArray readData1 = send(m_com5, buffer);
}

void COMPortSender::Remote_OFF()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x10;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x37;
    QByteArray readData1 = send(m_com6, buffer);
    QByteArray readData2 = send(m_com5, buffer);
}

void COMPortSender::com6ON()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power on
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x01;
    buffer[5] = 0x01;
    buffer[6] = 0x29;
    QByteArray readData1 = send(m_com6, buffer);
}

void COMPortSender::com6OFF()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x28;
    QByteArray readData1 = send(m_com6, buffer);
}

void COMPortSender::com5ON()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power on
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x01;
    buffer[5] = 0x01;
    buffer[6] = 0x29;

    QByteArray readData1 = send(m_com5, buffer);
}

void COMPortSender::com5OFF()
{
    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x28;
    QByteArray readData1 = send(m_com5, buffer);
}

int COMPortSender::readcom5U()
{
    QByteArray buffer(5, 0);
    buffer[0] = 0x75;
    buffer[1] = 0x00;
    buffer[2] = 0x47;
    buffer[3] = 0x00;
    buffer[4] = 0xbc;

    QByteArray readData = send(m_com5, buffer);

    uint8_t uu1, uu2;
    m_er2 = (readData[4] >> 4);
    uu1 = readData[5];
    uu2 = readData[6];
    double uu = (uu1 << 8) | uu2;
    uu = uu * 42 /(256);
    uu1 = readData[7];
    uu2 = readData[8];
    m_ii2 = (uu1 << 8) | uu2;
    m_ii2 = m_ii2 * 10 / 256;
    return uu;
}

int COMPortSender::readcom5I()
{
    return m_ii1;
}

int COMPortSender::readerr4I()
{
    return m_er2;
}

int COMPortSender::readerr11I()
{
    return m_er1;
}

int COMPortSender::readcom6U()
{
    QByteArray buffer(5, 0);
    buffer[0] = 0x75;
    buffer[1] = 0x00;
    buffer[2] = 0x47;
    buffer[3] = 0x00;
    buffer[4] = 0xbc;
    QByteArray readData = send(m_com6, buffer);
    uint8_t uu1, uu2;
    m_er1 = (readData[4] >> 4);
    uu1 = readData[5];
    uu2 = readData[6];
    double uu = (uu1 << 8) | uu2;
    uu = uu * 42 /(256);
    uu1 = readData[7];
    uu2 = readData[8];
    m_ii1 = (uu1 << 8) | uu2;
    m_ii1 = m_ii1 * 10 / 256;
    return uu;
}

int COMPortSender::readcom6I()
{
    return m_ii1;
}

void COMPortSender::setUIcom5(double u)
{
    QByteArray buffer(7, 0);
    uint32_t valU = (u * 25600) / 42;//U
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x32;
    buffer[3] = (valU >> 8) & 0xFF;
    buffer[4] = valU & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] =((sum >> 8) & 0xFF);
    buffer[6] = (sum  & 0xFF);

    QByteArray readData = send(m_com5, buffer);

    uint32_t vali = (155 * 25600 / u) / 10;//I
    if(vali > 25600)
    {
        vali = 25600;
    }

    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x33;
    buffer[3] = (vali >> 8) & 0xFF;
    buffer[4] = vali & 0xFF;
    sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] =((sum >> 8) & 0xFF);
    buffer[6] = (sum  & 0xFF);
    QByteArray readData1 = send(m_com5, buffer);
}

void COMPortSender::setUIcom6(double u)
{
    QByteArray buffer(7, 0);
    uint16_t sum = 0;
    uint32_t valU = (u * 25600) / 42;//U
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x32;
    buffer[3] = (valU >> 8) & 0xFF;
    buffer[4] = valU & 0xFF;

    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] = ((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);

    QByteArray readData1 = send(m_com6, buffer);

    uint32_t vali = (155 * 25600 / u) / 10;//I
    if (vali > 25600)
    {
        vali = 25600;
    }

    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x33;
    buffer[3] = (vali >> 8) & 0xFF;
    buffer[4] = vali & 0xFF;
    sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] = ((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);

    QByteArray readData2 = send(m_com6, buffer);
}

void COMPortSender::setoverUIcom5(double u, double ii)
{
    QByteArray buffer(7, 0);
    uint32_t valU = (u * 25600) / 42;//U
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x26;
    buffer[3] = (valU >> 8) & 0xFF;
    buffer[4] = valU & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] = ((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);
    QByteArray readData = send(m_com5, buffer);

    uint32_t vali = (ii * 25600) / 10;//I
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x27;
    buffer[3] = (vali >> 8) & 0xFF;
    buffer[4] = vali & 0xFF;
    sum = 0;

    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] =((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);

    QByteArray readData1 = send(m_com5, buffer);
}

void COMPortSender::setoverUIcom6(double u,double ii)
{
    QByteArray buffer(7, 0);
    uint32_t valU = ((u + 0.1) * 25600) / 42;//U
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x26;
    buffer[3] = (valU >> 8) & 0xFF;
    buffer[4] = valU & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] =((sum >> 8) & 0xFF);
    buffer[6] = (sum  & 0xFF);

    QByteArray readData1 = send(m_com6, buffer);
    uint32_t vali = (ii * 25600) / 10;//I
    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = 0x27;
    buffer[3] = (vali >> 8) & 0xFF;
    buffer[4] = vali & 0xFF;
    sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] =((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);
    QByteArray readData2 = send(m_com6, buffer);
}

int COMPortSender::id_stm()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x01;
    QByteArray readData1 = send(m_com4, buffer);

    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x02;
    QByteArray readData2 = send(m_com4, buffer);

    if(readData1[2] == readData2[2] && readData1[3] == readData2[3])
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
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x01;
    QByteArray readData1 = send(m_com8, buffer);

    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x02;

    QByteArray readData2 = send(m_com8, buffer);

    if(readData1[2] == readData2[2] && readData1[3] == readData2[3])
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
    if(m_flag_res_stm == 1)
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = 0x22;
        buffer[1] = 0x02;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData1 = send(m_com4, buffer);
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
    if(m_flag_res_tech==1)
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = 0x56;
        buffer[1] = 0x02;
        buffer[2] = 0x00;
        buffer[3] = 0x00;

        QByteArray readData1 = send(m_com8, buffer);
        uint8_t x=readData1[2];
        uint8_t z1, z2, z3;
        z1=x>>7;
        z2=x<<1;
        z2=z2>>7;
        z3=x<<2;
        z3=z3>>7;
        if(z1 == 0)
            res+=" Технол. модуль не готов к работе! \n";
        if(z2 == 1)
            res+=" Ошибки у Технол. модуля! \n";
        if(z3 == 1)
            res+=" Модуль Технол. после перезагрузки! \n";
        if(readData1.at(3)==0x10)
            res+=" Потеря байта из-за переполнения буфера RS485! \n";
        return res;
    }
    return "";
}

int COMPortSender::res_err_stm()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x03;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com4, buffer);
    return readData1[3];
}

int COMPortSender::res_err_tech()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x56;
    buffer[1] = 0x03;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com8, buffer);
    return readData1[3];
}

int COMPortSender::res_stm()
{
    m_flag_res_stm = 0;
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x04;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com4, buffer);

    if (m_com4 != Q_NULLPTR && m_com4->isOpen())
    {
        m_com4->close();
        m_com4->deleteLater();
        m_com4 = Q_NULLPTR;
    }

    for(int i = 0; i < 300; i++) // i guess there is some sort of govnomagics
    {
        Sleep(10);
        QApplication::processEvents();
    }

    m_com4 = createPort("com4");

    m_flag_res_stm = 1;
    return readData1[3];
}

int COMPortSender::res_tech()
{
    m_flag_res_tech = 0;
    QByteArray buffer(4, 0);
    buffer[0] = 0x56;
    buffer[1] = 0x04;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com8, buffer);

    if (m_com8 != Q_NULLPTR && m_com8->isOpen())
    {
        m_com8->close();
        m_com8->deleteLater();
        m_com8 = Q_NULLPTR;
    }

    for(int i = 0; i < 400; i++) // i guess there is some sort of govnomagics
    {
        Sleep(10);
        QApplication::processEvents();
    }

    m_com8 = createPort("com8");
    m_flag_res_tech = 1;
    return readData1[3];
}

int COMPortSender::fw_stm()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x22;
    buffer[1] = 0x06;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray readData1 = send(m_com4, buffer);
    return (readData1[2] * 10 + readData1[3]);
}

int COMPortSender::fw_tech()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0x56;
    buffer[1] = 0x06;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    QByteArray readData1 = send(m_com8, buffer);
    return (readData1[2] * 10 + readData1[3]);
}

