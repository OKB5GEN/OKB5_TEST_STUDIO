#include "Headers/system/system_state.h"

#include <QtSerialPort/QtSerialPort>

namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
}

SystemState::SystemState(QObject* parent):
    VariableController(parent)
{
}

SystemState::~SystemState()
{

}

void SystemState::init()
{
    // 1. Create COM-ports
    // 2. Send command "Get module address" (current and default) to each of them
    // 3. Depending on list, create modules and put port pointers to them
    // 4. Call "init" of the each module

    QSerialPort *port = createPort();

    if (port->isOpen())
    {
        int i = 0;
    }
    else
    {
        qDebug("No QSerialPort created"); // TODO critical error
    }
}

QSerialPort * SystemState::createPort()
{
    QSerialPort *port = new QSerialPort(this);
    port->open(QIODevice::ReadWrite);
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data5);
    port->setParity(QSerialPort::OddParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    return port;
}
