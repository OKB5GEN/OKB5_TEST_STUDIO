#include "Headers/system/com_port_module.h"
#include "Headers/logger/Logger.h"

#include <QtSerialPort>
#include <QApplication>
#include <windows.h>

namespace
{
    static const int WAIT_TIME = 100; // msec
}

COMPortModule::COMPortModule(QObject* parent):
    AbstractModule(parent),
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

//    QString str (request.toHex().toStdString().c_str());
    LOG_ERROR("Can not send request: %s", request.toHex().toStdString().c_str());
    return false;
}

bool COMPortModule::init()
{
    QString portName;

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    foreach (QSerialPortInfo info, ports)
    {
        if (   !info.hasVendorIdentifier()
            || !info.hasProductIdentifier()
            || info.productIdentifier() != mID.productId
            || info.vendorIdentifier() != mID.vendorId
            || info.description() != mID.description
            || info.serialNumber() != mID.serialNumber
           )
        {
            continue;
        }

        portName = info.portName();
        break;
    }

    if (portName.isNull())
    {
        return false;
    }

    createPort(portName);

    return postInit();
}

void COMPortModule::createPort(const QString& portName)
{
    mPort = new QSerialPort(portName);
    if (mPort->open(QIODevice::ReadWrite))
    {
        mPort->setBaudRate(QSerialPort::Baud115200);
        mPort->setDataBits(QSerialPort::Data8);
        mPort->setParity(QSerialPort::OddParity);
        mPort->setStopBits(QSerialPort::OneStop);
        mPort->setFlowControl(QSerialPort::NoFlowControl);
    }
    else
    {
        LOG_ERROR("Port name '%s' not opened. Error: '%s'", portName, mPort->errorString());
    }
}

void COMPortModule::resetError()
{

}

void COMPortModule::resetPort()
{
    QString portName = mPort->portName();

    if (mPort)
    {
        mPort->close();
        mPort->deleteLater();
        mPort = Q_NULLPTR;
    }

    //TODO possibly just open-close is enough without deletion
    for(int i = 0; i < 500; i++) // i guess some sort of govnomagics here "freeze app for 5 seconds to restore COM port with module after reset"
    {
        Sleep(10);
        QApplication::processEvents();
    }

    createPort(portName);
}

const COMPortModule::Identifier& COMPortModule::id() const
{
    return mID;
}

void COMPortModule::setId(const Identifier& id)
{
    mID = id;
}
