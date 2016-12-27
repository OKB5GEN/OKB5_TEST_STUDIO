#include "Headers/system/module.h"

#include <QtSerialPort>

namespace
{
    static const int WAIT_TIME = 100; // msec

}

Module::Module(QObject* parent):
    QObject(parent),
    mPort(Q_NULLPTR)
{
}

Module::~Module()
{

}

bool Module::send(const QByteArray& request, QByteArray& response)
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

void Module::setPort(QSerialPort* port)
{
    mPort = port;
}
