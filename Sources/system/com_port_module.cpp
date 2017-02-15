#include "Headers/system/com_port_module.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QtSerialPort>

namespace
{
    static const int DEFAULT_RESPONSE_WAIT_TIME = 10000;  // msec, TODO move to config file
    static const int DEFAULT_SEND_REQUEST_INTERVAL = 100; // msec, TODO move to config file
    static const int SOFT_RESET_UPDATE_TIME = 500;        // msec, TODO move to config file
}

COMPortModule::COMPortModule(QObject* parent):
    AbstractModule(parent),
    mPort(Q_NULLPTR),
    mResponseWaitTime(DEFAULT_RESPONSE_WAIT_TIME),
    mSendInterval(DEFAULT_SEND_REQUEST_INTERVAL)
{
    mResponseWaitTimer = new QTimer(this);
    mResponseWaitTimer->setSingleShot(true);
    connect(mResponseWaitTimer, SIGNAL(timeout()), this, SLOT(onResponseTimeout()));

    mSendTimer = new QTimer(this);
    mSendTimer->setSingleShot(true);
    connect(mSendTimer, SIGNAL(timeout()), this, SLOT(sendRequest()));

    mSoftResetTimer = new QTimer(this);
    mSoftResetTimer->setSingleShot(true);
    connect(mSoftResetTimer, SIGNAL(timeout()), this, SLOT(tryCreatePort()));
}

COMPortModule::~COMPortModule()
{
    mResponseWaitTimer->stop();
    mSendTimer->stop();

    if (mPort && mPort->isOpen())
    {
        mPort->close();
    }
}

bool COMPortModule::sendToPort(const QByteArray& request)
{
    if (!mPort)
    {
        LOG_ERROR(QString("Send data to COM port failed. No port created!"));
        return false;
    }

    if (!mPort->isOpen())
    {
        LOG_ERROR(QString("Send data to %1 failed! Port is closed").arg(mPort->portName()));
        return false;
    }

    qint64 bytesWritten = mPort->QIODevice::write(request);
    if (bytesWritten == -1)
    {
        LOG_ERROR(QString("Send data to %1 failed! Port error: %2").arg(mPort->portName()).arg(mPort->errorString()));
        return false;
    }

    if (!mPort->waitForBytesWritten(-1))
    {
        //Returns true if a payload of data was written to the device;
        //otherwise returns false (i.e. if the operation timed out, or if an error occurred).
        LOG_ERROR(QString("Send data to %1 failed! Payload not written! Port error: %2").arg(mPort->portName()).arg(mPort->errorString()));
        return false;
    }

    return true;
}

QString COMPortModule::findPortName() const
{
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

        return info.portName();
    }

    return QString();
}

void COMPortModule::initialize()
{
    QString portName = findPortName();

    if (portName.isNull())
    {
        emit initializationFinished(QString("Configuration error. No COM port module found"));
        return;
    }

    QString error = createPort(portName);

    if (!error.isEmpty())
    {
        emit initializationFinished(error);
        return;
    }

    initializeCustom();
}

QString COMPortModule::createPort(const QString& portName)
{
    QString error;

    mPort = new QSerialPort(portName);
    if (mPort->open(QIODevice::ReadWrite))
    {
        mPort->setBaudRate(QSerialPort::Baud115200);
        mPort->setDataBits(QSerialPort::Data8);
        mPort->setParity(QSerialPort::OddParity);
        mPort->setStopBits(QSerialPort::OneStop);
        mPort->setFlowControl(QSerialPort::NoFlowControl);
        connect(mPort, SIGNAL(readyRead()), this, SLOT(onResponseReceived()));
    }
    else
    {
        error = QString("Port %1 not opened. Port error: %2").arg(mPort->portName()).arg(mPort->errorString());
    }

    return error;
}

void COMPortModule::onResponseReceived()
{
    mResponseWaitTimer->stop();

    if (!mPort || !mPort->isOpen())
    {
        LOG_ERROR(QString("INTERNAL ERROR"));
        return;
    }

    QByteArray response = mPort->readAll();
    LOG_INFO(QString("Recv <---- %1: %2").arg(mPort->portName()).arg(QString(response.toHex().toStdString().c_str())));

    if (!processResponse(mRequestQueue.front().operation, mRequestQueue.front().data, response))
    {
        LOG_ERROR(QString("%1 response processing error. Flushing request queue...").arg(mPort->portName()));
        mRequestQueue.clear();
        return;
    }

    mRequestQueue.pop_front();

    if (mRequestQueue.empty())
    {
        onTransmissionComplete();
    }
    else
    {
        mSendTimer->start(mSendInterval);
    }
}

void COMPortModule::addRequest(uint32_t operationID, const QByteArray& request)
{
    if (request.isEmpty())
    {
        LOG_ERROR(QString("Empty request try to send to %1 port").arg(mPort->portName()));
        onTransmissionError(operationID);
        return;
    }

    if (mRequestQueue.empty())
    {
        mSendTimer->start(mSendInterval);
    }

    Request r;
    r.data = request;
    r.operation = operationID;
    mRequestQueue.push_back(r);
}

void COMPortModule::onResponseTimeout()
{
    LOG_ERROR(QString("%1 wait for response timeout. Flushing request queue...").arg(mPort->portName()));
    uint32_t operationID = mRequestQueue.front().operation;
    mRequestQueue.clear();
    onTransmissionError(operationID);
}

void COMPortModule::sendRequest()
{
    if (mRequestQueue.isEmpty())
    {
        LOG_ERROR(QString("Can not send request to %1. Queue is empty").arg(mPort->portName()));
        return;
    }

    if (!sendToPort(mRequestQueue.front().data))
    {
        onTransmissionError(mRequestQueue.front().operation);
        LOG_ERROR(QString("%1 transmission error occured. Flushing request queue...").arg(mPort->portName()));
        mRequestQueue.clear();
        return;
    }

    LOG_INFO(QString("Send ----> %1: %2").arg(mPort->portName()).arg(QString(mRequestQueue.front().data.toHex().toStdString().c_str())));
    mResponseWaitTimer->start(mResponseWaitTime);
}

void COMPortModule::setSendInterval(int msec)
{
    if (msec <= 0)
    {
        LOG_ERROR(QString("Invalid value: %1").arg(msec));
        return;
    }

    mSendInterval = msec;
}

void COMPortModule::setResponseWaitTime(int msec)
{
    if (msec <= 0)
    {
        LOG_ERROR(QString("Invalid value: %1").arg(msec));
        return;
    }

    mResponseWaitTime = msec;
}

void COMPortModule::softReset()
{
    if (!mPort || !mPort->isOpen())
    {
        LOG_WARNING("Trying to reset inactive port");
        return;
    }

    LOG_INFO(QString("%1 soft reset started ..."));

    disconnect(mPort, SIGNAL(readyRead()), this, SLOT(onResponseReceived()));
    mPort->close();
    mPort->deleteLater();
    mPort = Q_NULLPTR;

    mSoftResetTimer->start(SOFT_RESET_UPDATE_TIME);
}

void COMPortModule::tryCreatePort()
{
    QString portName = findPortName();

    if (portName.isNull())
    {
        LOG_INFO(QString("Module still not active. Restarting update timer ..."));
        mSoftResetTimer->start(SOFT_RESET_UPDATE_TIME);
        return;
    }

    QString error = createPort(portName);

    if (!error.isEmpty())
    {
        LOG_ERROR(error);
        return;
    }

    LOG_INFO(QString("%1 is up after soft reset").arg(mPort->portName()));
    onSoftResetComplete();
}

const COMPortModule::Identifier& COMPortModule::id() const
{
    return mID;
}

void COMPortModule::setId(const Identifier& id)
{
    mID = id;
}
