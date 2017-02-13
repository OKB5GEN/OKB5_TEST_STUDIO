#include "Headers/system/com_port_module.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QtSerialPort>
#include <QApplication> // TODO remove
#include <windows.h> // TODO remove

namespace
{
    static const int PROTECTION_TIMEOUT = 10000; // msec TODO move to config file
}

COMPortModule::COMPortModule(QObject* parent):
    AbstractModule(parent),
    mPort(Q_NULLPTR),
    mIsInitialized(false)
{
    mProtectionTimer = new QTimer(this);
    mProtectionTimer->setSingleShot(true);

    connect(mProtectionTimer, SIGNAL(timeout()), this, SLOT(onResponseTimeout()));
}

COMPortModule::~COMPortModule()
{
    mProtectionTimer->stop();

    if (mPort && mPort->isOpen())
    {
        mPort->close();
    }
}

bool COMPortModule::send(const QByteArray& request)
{
    if (!mPort)
    {
        LOG_ERROR("Send data to COM port failed! No port created!");
        return false;
    }

    if (!mPort->isOpen())
    {
        LOG_ERROR("Send data to COM port failed! Error: %s",  mPort->errorString().toStdString().c_str());
        return false;
    }

    qint64 bytesWritten = mPort->QIODevice::write(request);
    if (bytesWritten == -1)
    {
        LOG_ERROR("Send data to COM port failed! No data written! Error: %s",  mPort->errorString().toStdString().c_str());
        return false;
    }

    if (!mPort->waitForBytesWritten(-1))
    {
        //Returns true if a payload of data was written to the device;
        //otherwise returns false (i.e. if the operation timed out, or if an error occurred).
        LOG_ERROR("Send data to COM port failed! Payload not written! Error: %s",  mPort->errorString().toStdString().c_str());
        return false;
    }

    LOG_TRACE("Send data to COM port:  %s", request.toHex().toStdString().c_str());
    mProtectionTimer->start(PROTECTION_TIMEOUT);
    return true;
}

bool COMPortModule::initialize()
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
        LOG_ERROR(QString("No COM port name '%1' module found").arg(portName));
        return false;
    }

    createPort(portName);
    connect(mPort, SIGNAL(readyRead()), this, SLOT(onResponseReceived()));
    mIsInitialized = postInit();

    return mIsInitialized;
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
        LOG_ERROR("Port name '%s' not opened. Error: '%s'", portName.toStdString().c_str(), mPort->errorString().toStdString().c_str());
    }
}

void COMPortModule::resetError()
{

}

void COMPortModule::resetPort()
{
    int TODO; // refactor
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

void COMPortModule::onResponseReceived()
{
    mProtectionTimer->stop();

    if (mPort && mPort->isOpen())
    {
        QByteArray response;
        response.append(mPort->readAll());
        processResponse(response);
    }
}

void COMPortModule::onResponseTimeout()
{
    LOG_ERROR(QString("Wait for response timeout. "));

    // send empty response
    QByteArray response;
    processResponse(response);
}
