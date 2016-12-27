#include "Headers/system/OTD.h"
#include "Headers/gui/mainwindow.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QString>
#include "qapplication.h"
#include <QtSerialPort/QtSerialPort>
#include "synchapi.h"

OTD::OTD(QString s, QObject* parent) :
    QObject(parent),
    name(s)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(OTD_timer()));
}

QByteArray OTD::send(QByteArray data, double readTimeout, double delayBeforeRecv /*= 0*/)
{
    QByteArray readData;
    if (m_port && m_port->isOpen())
    {
        m_port->QIODevice::write(data);
        m_port->waitForBytesWritten(-1);

        if (delayBeforeRecv > 0)
        {
            Sleep(delayBeforeRecv);
        }

        readData = m_port->readAll();
        while (m_port->waitForReadyRead(readTimeout))
        {
            readData.append(m_port->readAll());
        }
    }

    return readData;
}

void OTD::COMConnectorOTD()
{
    m_port = new QSerialPort("com7");
    m_port->open(QIODevice::ReadWrite);
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data5);
    m_port->setParity(QSerialPort::OddParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    m_isActive = true;
    OTD_id();
    OTDtemper();
}

void OTD::OTD_id()
{
    QByteArray bw(4, 0);
    bw[0] = 0xff;
    bw[1] = 0x01;
    bw[2] = 0x00;
    bw[3] = 0x01;
    QByteArray readData1 = send(bw, 100);

    bw[0] = 0xff;
    bw[1] = 0x01;
    bw[2] = 0x00;
    bw[3] = 0x02;

    QByteArray readData0 = send(bw, 500);

    if(readData1[2] != readData0[2] || readData1[3] != readData0[3])
    {
        emit OTD_id1();
    }
}

void OTD::OTD_req()
{
    if(m_isActive)
    {
        QString res;
        QByteArray bw(4, 0);
        bw[0] = 0x44;
        bw[1] = 0x02;
        bw[2] = 0x00;
        bw[3] = 0x00;
        QByteArray readData1 = send(bw, 100);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        res = "";
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if (z1 == 0)
        {
            res += " ОТД не готов к работе! \n";
        }

        if (z2 == 1)
        {
            res += " Ошибки у модуля ОТД! \n";
        }

        if (z3 == 1)
        {
            res += " Модуль ОТД после перезагрузки! \n";
        }

        emit OTD_reqr (res);
    }
}

void OTD::OTD_fw()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x06;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 100);
    emit OTD_vfw(readData1[2] * 10 + readData1[3]);
}

void OTD::res_OTD()
{
    m_isActive = false;
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x04;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 100);
    COMCloseOTD();
    for(int i = 0; i < 400; i++) // TODO Remove this shit
    {
        Sleep(10);
        QApplication::processEvents();
    }

    COMConnectorOTD();
    emit OTD_res(readData1[3]);
}

void OTD::err_res_OTD()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x03;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 100);
    emit OTD_err_res(readData1[3]);
}

void OTD::OTDres1()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x26;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 500);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при перезагрузке!");
    }
}

void OTD::OTDres2()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x27;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 500);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при перезагрузке!");
    }
    else
    {
        emit err_OTD("");
    }
}

void OTD::OTDmeas1()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x28;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 500, 2000);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при запуске измерений 1-й линии!");
    }

    OTDtm1();
}

void OTD::OTDmeas2()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x29;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 500, 2000);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при запуске измерений 2-й линии!");
    }

    OTDtm2();
}

void OTD::OTDtemper()
{
    QString data;
    data += "Кол-во датчиков DS1820 по оси 1: ";
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x1e;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData0 = send(bw, 500);
    m_sensorsCntAxis1 = readData0[2];
    data += QString::number(m_sensorsCntAxis1);
    data += "\n";
    data += "Адреса датчиков по оси 1: \n";

    for(int j = 1; j <= m_sensorsCntAxis1; j++)
    {
        data += QString::number(j);
        data += " : ";
        for(int k = 0; k < 8; k++)
        {
            bw[0] = 0x44;
            bw[1] = 0x2a;
            bw[2] = j;
            bw[3] = k;
            QByteArray readData3 = send(bw, 100);
            data += QString::number(readData3[2], 16);
        }

        data += "\n";
    }

    data += "\n";
    if(readData0.at(3) == 2)
    {
        data += "Ошибка при считывании датчиков 1-й оси\n";
    }

    data += "Кол-во датчиков DS1820 по оси 2: ";
    bw[0] = 0x44;
    bw[1] = 0x1d;
    bw[2] = 0x00;
    bw[3] = 0x00;

    QByteArray readData3 = send(bw, 500);
    m_sensorsCntAxis2 = readData3[2];
    data += QString::number(m_sensorsCntAxis2);
    data += "\n";
    data += "Адреса датчиков по оси 2: \n";
    for(int j = 1; j <= m_sensorsCntAxis2; j++)
    {
        data += QString::number(j);
        data += " : ";
        for(int k = 0; k < 8; k++)
        {
            bw[0] = 0x44;
            bw[1] = 0x2a;
            bw[2] = j;
            bw[3] = k;
            QByteArray readData4 = send(bw, 100);
            data += QString::number(readData4[2], 16);
        }

        data += "\n";
    }

    if (readData3.at(3) == 2)
    {
        data += "Ошибка при считывании датчиков 2-й оси\n";
    }

    emit temp_OTD(data);
}

void OTD::OTDtm1()
{
    QString temp;
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x1f;
    bw[3] = 0x00;

    for(int i = 1; i <= m_sensorsCntAxis1; i++)
    {
        temp += " Температура датчиков 1-й линии \n ";
        temp += QString::number(i);
        temp += " : ";
        bw[2] = i;
        QByteArray readData1 = send(bw, 500);
        uint8_t uu1, uu2, z;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double uu = (uu1 << 8) | uu2;
        uint8_t x = readData1[2];
        z = x << 4;
        z = z >> 7;
        if (z == 0)
        {
            uu = uu / 16;
        }

        if (z == 1)
        {
            uu = (uu - 4096) / 16;
        }

        temp += QString::number(uu);
        temp += "\n";
    }

    emit tm_OTD1(temp);
}

void OTD::OTDtm2()
{
    QString temp;
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x20;
    bw[3] = 0x00;

    for(int i = 1; i <= m_sensorsCntAxis2; i++)
    {
        temp += " Температура датчиков 2-й линии \n ";
        temp += QString::number(i);
        temp += " : ";
        bw[2] = i;
        QByteArray readData1 = send(bw, 500);
        uint8_t uu1, uu2, z;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double uu = (uu1 << 8) | uu2;
        uint8_t x = readData1[2];
        z = x << 4;
        z = z >> 7;
        if (z == 0)
        {
            uu = uu / 16;
        }

        if (z == 1)
        {
            uu = (uu - 4096) / 16;
        }

        temp += QString::number(uu);
        temp += "\n";
    }

    emit tm_OTD2(temp);
}

void OTD::OTDPT()
{
    QByteArray bw(4, 0);
    bw[0] = 0x44;
    bw[1] = 0x1c;
    bw[2] = 0x01;
    bw[3] = 0x00;
    QByteArray readData1 = send(bw, 500);
    uint8_t uu1, uu2;
    uu1 = readData1[2];
    uu2 = readData1[3];
    double uu = (uu1 << 8) | uu2;
    uu = uu / 32 - 256;

    bw[0] = 0x44;
    bw[1] = 0x1c;
    bw[2] = 0x02;
    bw[3] = 0x00;
    QByteArray readData2 = send(bw, 500);
    uu1 = readData2[2];
    uu2 = readData2[3];
    double uu3 = (uu1 << 8) | uu2;
    uu3 = uu3 / 32 - 256;
    emit start_OTDPT(uu * 100, uu3 * 100);
}

void OTD::OTD_avt(int x, int y)
{
    if(x == 1)
    {
        m_timer->start(y);
    }
    else
    {
        y = 0;
    }
}

void OTD::OTD_timer()
{
    OTDPT();
    OTDmeas1();
    OTDmeas2();
}

void OTD::COMCloseOTD()
{
    if (m_port)
    {
        m_port->close();
    }
}

void OTD::doWork()
{
    emit start_OTD();
}