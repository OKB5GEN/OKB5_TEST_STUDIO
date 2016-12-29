#include "Headers/system/com_port_module.h"

#include <QtSerialPort>
#include <QApplication>
#include <windows.h>

namespace
{
    static const int WAIT_TIME = 100; // msec
}

COMPortModule::COMPortModule(QObject* parent):
    QObject(parent),
    mPort(Q_NULLPTR)
{
}

COMPortModule::~COMPortModule()
{
    if (mPort && mPort->isOpen())
    {
        mPort->close();
    }
}

bool COMPortModule::send(const QByteArray& request, QByteArray& response)
{
    if (mPort && mPort->isOpen())
    {
        mPort->QIODevice::write(request);
        mPort->waitForBytesWritten(-1);

        response = mPort->readAll();
        while (mPort->waitForReadyRead(WAIT_TIME))
        {
            response.append(mPort->readAll());
        }

        return true;
    }

    return false;
}

bool COMPortModule::init()
{
    createPort();

    return postInit();
}

void COMPortModule::createPort()
{
    mPort = new QSerialPort();
    mPort->open(QIODevice::ReadWrite);
    mPort->setBaudRate(QSerialPort::Baud115200);
    mPort->setDataBits(QSerialPort::Data5);
    mPort->setParity(QSerialPort::OddParity);
    mPort->setStopBits(QSerialPort::OneStop);
    mPort->setFlowControl(QSerialPort::NoFlowControl);
}

void COMPortModule::resetError()
{

}

void COMPortModule::resetPort()
{
    if (mPort)
    {
        mPort->close();
        mPort->deleteLater();
        mPort = Q_NULLPTR;
    }

    for(int i = 0; i < 500; i++) // i guess some sort of govnomagics here "freeze app for 5 seconds to restore COM port with module after reset"
    {
        Sleep(10);
        QApplication::processEvents();
    }

    createPort();
}
