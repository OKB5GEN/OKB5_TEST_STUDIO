#include "Headers/system/com_port_module.h"
#include "Headers/logger/Logger.h"

#include <QTimer>

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
        LOG_ERROR(QString("Send data to %1 failed. No port created!").arg(moduleName()));
        return false;
    }

    if (!mPort->isOpen())
    {
        LOG_ERROR(QString("Send data to %1 failed! %2 port is closed").arg(moduleName()).arg(mPort->portName()));
        return false;
    }

    qint64 bytesWritten = mPort->QIODevice::write(request);
    if (bytesWritten == -1)
    {
        LOG_ERROR(QString("Send data to %1 failed! %2 port error: %3").arg(moduleName()).arg(mPort->portName()).arg(mPort->errorString()));
        return false;
    }

    if (!mPort->waitForBytesWritten(-1))
    {
        //Returns true if a payload of data was written to the device;
        //otherwise returns false (i.e. if the operation timed out, or if an error occurred).
        LOG_ERROR(QString("Send data to %1 failed! Payload not written! %2 port error: %3").arg(moduleName()).arg(mPort->portName()).arg(mPort->errorString()));
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

void COMPortModule::onApplicationStart()
{
    setModuleState(AbstractModule::INITIALIZING);

    QString portName = findPortName();

    if (portName.isNull())
    {
        setModuleState(AbstractModule::INITIALIZED_FAILED, QString("Module %1 configuration error. No COM port module found").arg(moduleName()));
        return;
    }

    QString error = createPort(portName);

    if (!error.isEmpty())
    {
        setModuleState(AbstractModule::INITIALIZED_FAILED, error);
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
        //connect(mPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(onErrorOccured(QSerialPort::SerialPortError)));
    }
    else
    {
        error = QString("%1 port not created. %2 port error: %3").arg(moduleName()).arg(mPort->portName()).arg(mPort->errorString());
    }

    return error;
}

void COMPortModule::onErrorOccured(QSerialPort::SerialPortError error)
{
    QMetaEnum e = QMetaEnum::fromType<QSerialPort::SerialPortError>();
    LOG_ERROR(QString("%1 (%2) module QSerialPort error occured: %3").arg(moduleName()).arg(mPort->portName()).arg(e.valueToKey(error)));
}

void COMPortModule::onResponseReceived()
{
    mResponseWaitTimer->stop();

    if (!mPort || !mPort->isOpen())
    {
        LOG_ERROR(QString("%1 module INTERNAL ERROR").arg(moduleName()));
        return;
    }

    QByteArray response = mPort->readAll();
    LOG_INFO(QString("Recv <---- %1 (%2): %3").arg(moduleName()).arg(mPort->portName()).arg(QString(response.toHex().toStdString().c_str())));

    if (!processResponse(mRequestQueue.front().operation, mRequestQueue.front().data, response))
    {
        LOG_ERROR(QString("%1 (%2) response processing error. Flushing request queue...").arg(moduleName()).arg(mPort->portName()));
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
    if (!mPort || !mPort->isOpen())
    {
        LOG_ERROR(QString("Can not add request. Module %1 not ready").arg(moduleName()));
        onTransmissionError(operationID);
        return;
    }

    if (request.isEmpty())
    {
        LOG_ERROR(QString("Empty request try to send to %1 (%2)").arg(moduleName()).arg(mPort->portName()));
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
    uint32_t operationID = mRequestQueue.front().operation;

    bool resend = false;//TODO resend message or abort transmission?

    if (resend)
    {
        LOG_ERROR(QString("%1 (%2) wait for response timeout. Try to resend...").arg(moduleName()).arg(mPort->portName()));
        mSendTimer->start(mSendInterval); // try to resend?
    }
    else
    {
        LOG_ERROR(QString("%1 (%2) wait for response timeout. Flushing request queue...").arg(moduleName()).arg(mPort->portName()));
        mRequestQueue.clear();
        onTransmissionError(operationID);
    }
}

void COMPortModule::sendRequest()
{
    if (mRequestQueue.isEmpty())
    {
        LOG_ERROR(QString("Can not send request to %1 (%2). Queue is empty").arg(moduleName()).arg(mPort->portName()));
        return;
    }

    if (!sendToPort(mRequestQueue.front().data))
    {
        onTransmissionError(mRequestQueue.front().operation);
        LOG_ERROR(QString("%1 (%2) transmission error occured. Flushing request queue...").arg(moduleName()).arg(mPort->portName()));
        mRequestQueue.clear();
        return;
    }

    LOG_INFO(QString("Send ----> %1 (%2): %3").arg(moduleName()).arg(mPort->portName()).arg(QString(mRequestQueue.front().data.toHex().toStdString().c_str())));
    mResponseWaitTimer->start(mResponseWaitTime);

    //TODO this is some hack. without it readyRead() signal is not emitted or emitted randomly true random :)
    if (mPort->waitForReadyRead(1))
    {
        LOG_DEBUG(QString("Bytes available %1").arg(mPort->bytesAvailable()));
    }
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
    if (!mPort || !mPort->isOpen() /*|| !mModuleReady*/) //TODO check moduleState(), TODO is module can be soft resetted? (power modules can not be?)
    {
        LOG_WARNING(QString("Trying to soft reset inactive module %1").arg(moduleName()));
        return;
    }

    LOG_INFO(QString("%1 (%2) soft reset started ...").arg(moduleName()).arg(mPort->portName()));

    disconnect(mPort, SIGNAL(readyRead()), this, SLOT(onResponseReceived()));
    //disconnect(mPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(onErrorOccured(QSerialPort::SerialPortError)));
    mPort->close();
    mPort->deleteLater();
    mPort = Q_NULLPTR;
    setModuleState(AbstractModule::SOFT_RESETTING);

    mSoftResetTimer->start(SOFT_RESET_UPDATE_TIME);
}

void COMPortModule::tryCreatePort()
{
    QString portName = findPortName();

    if (portName.isNull())
    {
        LOG_INFO(QString("Module %1 still not active. Restarting update timer ...").arg(moduleName()));
        mSoftResetTimer->start(SOFT_RESET_UPDATE_TIME);
        return;
    }

    QString error = createPort(portName);

    if (!error.isEmpty())
    {
        LOG_ERROR(error);
        return;
    }

    LOG_INFO(QString("%1 (%2) is up after soft reset").arg(moduleName()).arg(mPort->portName()));
    onApplicationStart();
}

const COMPortModule::Identifier& COMPortModule::id() const
{
    return mID;
}

void COMPortModule::setId(const Identifier& id)
{
    mID = id;
}
