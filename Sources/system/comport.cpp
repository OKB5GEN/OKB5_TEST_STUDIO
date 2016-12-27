#include "Headers/system/comport.h"

#include <QObject>
#include <QString>
#include "qapplication.h"
#include "synchapi.h"

namespace
{
    static const uint8_t MAX_VOLTAGE = 42; // volts
    static const uint8_t MAX_CURRENT = 10; // ampers
    static const uint8_t MAX_POWER = 155; // watts actually 160, but for safety purposes reduced to 155

    static const uint8_t STM_DEFAULT_ADDR = 0x22;
    static const uint8_t TECH_DEFAULT_ADDR = 0x56;
}

COMPortSender::COMPortSender(QObject *parent):
    QObject(parent)
{
}

COMPortSender::~COMPortSender()
{
    foreach (ModuleInfo info, m_modules)
    {
        if (info.port->isOpen())
        {
            info.port->close();
        }
    }
}

void COMPortSender::createPorts()
{
    // TODO: The order of ports creation possibly important!
    {
        COMPortSender::ModuleInfo info;
        info.port = createPort("6");
        info.state = true;
        info.address = 0xFF;
        m_modules[POW_ANT_DRV] = info;
    }

    {
        COMPortSender::ModuleInfo info;
        info.port = createPort("5");
        info.state = true;
        info.address = 0xFF;
        m_modules[POW_ANT_DRV_CTRL] = info;
    }

    {
        COMPortSender::ModuleInfo info;
        info.port = createPort("4");
        info.state = true;
        info.address = STM_DEFAULT_ADDR;
        m_modules[STM] = info;
    }

    {
        COMPortSender::ModuleInfo info;
        info.port = createPort("8");
        info.state = true;
        info.address = TECH_DEFAULT_ADDR;
        m_modules[TECH] = info;
    }
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
}

QByteArray COMPortSender::send(QSerialPort * port, QByteArray data)
{
    QByteArray readData;
    if (port->isOpen())
    {
        port->QIODevice::write(data);
        port->waitForBytesWritten(-1);

        readData = port->readAll();
        while (port->waitForReadyRead(100))
        {
            readData.append(port->readAll());
        }
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
    QByteArray readData11 = send(getPort(POW_ANT_DRV_CTRL), buffer1);
    QByteArray readData21 = send(getPort(POW_ANT_DRV), buffer1);

    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x28;

    QByteArray readData1 = send(getPort(POW_ANT_DRV_CTRL), buffer);
    QByteArray readData2 = send(getPort(POW_ANT_DRV), buffer);

    setVoltageAndCurrent(POW_ANT_DRV, 0.5);
    setVoltageAndCurrent(POW_ANT_DRV_CTRL, 0.5);
}

int COMPortSender::setPowerChannelState(int channel, PowerState state)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = POWER_CHANNEL_CTRL;
    buffer[2] = channel;
    buffer[3] = (state == POWER_ON) ? 1 : 0;

    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int COMPortSender::stm_on_mko(int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = SET_MKO_PWR_CHANNEL_STATE;
    buffer[2] = x;
    buffer[3] = y;

    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int COMPortSender::stm_check_fuse(int fuse)
{
    QByteArray buffer(4, 0);
    buffer[0] = STM_DEFAULT_ADDR;
    buffer[1] = GET_PWR_MODULE_FUSE_STATE;
    buffer[2] = fuse;
    buffer[3] = 0x00;
    QByteArray readData1 = send(getPort(STM), buffer);
    return readData1[3];
}

int COMPortSender::tech_send(int com, int x, int y)
{
    QByteArray buffer(4, 0);
    buffer[0] = TECH_DEFAULT_ADDR;
    buffer[1] = com;
    buffer[2] = x;
    buffer[3] = y;
    QByteArray readData1 = send(getPort(TECH), buffer);
    return readData1[3];
}

int COMPortSender::tech_read(int x)
{
    if(isActive(TECH))
    {
        uint8_t command = 0;
        if(x == 1)
        {
            command = CHECK_RECV_DATA_RS485;
        }
        else
        {
            command = CHECK_RECV_DATA_CAN;
        }

        QByteArray buffer(4, 0);
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = command;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(getPort(TECH), buffer);

        uint8_t uu1,uu2;
        uu1 = readData2[2];
        uu2 = readData2[3];
        double uu = (uu1 << 8) | uu2;
        return uu;
    }

    return 0;
}

QString COMPortSender::tech_read_buf(int x, int len)
{
    uint8_t command = 0;
    if(x == 1)
    {
        command = RECV_DATA_RS485;
    }
    else
    {
        command = RECV_DATA_CAN;
    }

    QString result;
    QByteArray buffer(4, 0);
    for(int i = 0; i < len; i++)
    {
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = command;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData2 = send(getPort(TECH), buffer);

        if(readData2.at(2) == 1)
        {
            result += "em ";
        }

        if(readData2.at(2) == 2)
        {
            result += "uu ";
        }

        result += readData2[3];
        result += " ";
        QApplication::processEvents();
    }

    return result;
}

double COMPortSender::stm_data_ch(int ch)
{
    if (isActive(STM))
    {
        QByteArray buffer(4, 0);
        buffer[0] = STM_DEFAULT_ADDR;
        buffer[1] = GET_CHANNEL_TELEMETRY;
        buffer[2] = ch;
        buffer[3] = 0x00;

        QByteArray readData1 = send(getPort(STM), buffer);

        uint8_t uu1, uu2;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double res = (uu1 << 8) | uu2;
        return res;
    }

    return 50000;
}

QSerialPort* COMPortSender::getPort(ModuleID id)
{
    QSerialPort* port = m_modules.value(id, ModuleInfo()).port;
    Q_ASSERT(port != Q_NULLPTR);
    return port;
}

int COMPortSender::resetError(ModuleID id)
{
    QSerialPort * port = getPort(id);
    QByteArray buffer;

    switch (id)
    {
    case POW_ANT_DRV_CTRL:
    case POW_ANT_DRV:
        {
            buffer.resize(7);
            buffer[0] = 0xf1;
            buffer[1] = 0x00;
            buffer[2] = 0x36;
            buffer[3] = 0x0a;
            buffer[4] = 0x0a;
            buffer[5] = 0x01;
            buffer[6] = 0x3b;
        }
        break;

    case STM:
        {
            buffer.resize(4);
            buffer[0] = STM_DEFAULT_ADDR;
            buffer[1] = RESET_ERROR;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
        }
        break;

    case TECH:
        {
            buffer.resize(4);
            buffer[0] = TECH_DEFAULT_ADDR;
            buffer[1] = RESET_ERROR;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
        }
        break;
    default:
        break;
    }

    QByteArray readData = send(port, buffer);
    if (readData.size() > 3)
    {
        return readData[3];
    }

    return 0;
}

void COMPortSender::setPowerState(ModuleID id, PowerState state)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    QByteArray buffer(7, 0);
    buffer[0] = 0xf1;//power on/off
    buffer[1] = 0x00;
    buffer[2] = 0x36;
    buffer[3] = 0x01;
    buffer[4] = 0x01;
    buffer[5] = 0x01;
    buffer[6] = (state == POWER_ON) ? 0x29 : 0x28;
    QByteArray readData1 = send(port, buffer);
}

void COMPortSender::setPowerValue(uint8_t valueID, double value, double maxValue, QSerialPort * port)
{
    QByteArray buffer(7, 0);
    uint32_t val = uint32_t((value * 256 * 100) / maxValue);

    if(val > (256 * 100))
    {
        val = (256 * 100);
    }

    buffer[0] = 0xf1;
    buffer[1] = 0x00;
    buffer[2] = valueID;
    buffer[3] = (val >> 8) & 0xFF;
    buffer[4] = val & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = buffer[i];
        sum = (sum + s) & 0xFFFF;
    }

    buffer[5] = ((sum >> 8) & 0xFF);
    buffer[6] = (sum & 0xFF);
    QByteArray readData = send(port, buffer);
}

void COMPortSender::setMaxVoltageAndCurrent(ModuleID id, double voltage, double current)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    // TODO make constants
    setPowerValue(MAX_VOLTAGE_VAL, voltage, MAX_VOLTAGE, port);
    setPowerValue(MAX_CURRENT_VAL, current, MAX_CURRENT, port);
}

void COMPortSender::setVoltageAndCurrent(ModuleID id, double voltage)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    // TODO make constants
    setPowerValue(CUR_VOLTAGE_VAL, voltage, MAX_VOLTAGE, port);
    setPowerValue(CUR_CURRENT_VAL, ((double)MAX_POWER) / voltage, MAX_CURRENT, port);
}

void COMPortSender::getCurVoltageAndCurrent(ModuleID id, double& voltage, double& current, uint8_t& error)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == POW_ANT_DRV_CTRL || id == POW_ANT_DRV);

    QByteArray buffer(5, 0);
    buffer[0] = 0x75;
    buffer[1] = 0x00;
    buffer[2] = 0x47;
    buffer[3] = 0x00;
    buffer[4] = 0xbc;

    QByteArray readData = send(port, buffer);

    uint8_t uu1, uu2;
    error = (readData[4] >> 4);

    uu1 = readData[5];
    uu2 = readData[6];
    voltage = (uu1 << 8) | uu2;
    voltage = voltage * MAX_VOLTAGE / 256;

    uu1 = readData[7];
    uu2 = readData[8];
    current = (uu1 << 8) | uu2;
    current = current * MAX_CURRENT / 256;
}

int COMPortSender::id_stm()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x01;
    QByteArray readData1 = send(getPort(STM), buffer);

    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x02;
    QByteArray readData2 = send(getPort(STM), buffer);

    if(readData1[2] == readData2[2] && readData1[3] == readData2[3])
    {
        return 1;
    }

    return 0;
}

int COMPortSender::id_tech()
{
    QByteArray buffer(4, 0);
    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x01;
    QByteArray readData1 = send(getPort(TECH), buffer);

    buffer[0] = 0xff;
    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x02;

    QByteArray readData2 = send(getPort(TECH), buffer);

    if (readData1[2] == readData2[2] && readData1[3] == readData2[3])
    {
        return 1;
    }

    return 0;
}

QString COMPortSender::req_stm()
{
    if (isActive(STM))
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = STM_DEFAULT_ADDR;
        buffer[1] = 0x02;
        buffer[2] = 0x00;
        buffer[3] = 0x00;
        QByteArray readData1 = send(getPort(STM), buffer);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if(z1 == 0)
            res += " СТМ не готов к работе! \n";
        if(z2 == 1)
            res += " Ошибки у модуля СТМ! \n";
        if(z3 == 1)
            res += " Модуль СТМ после перезагрузки! \n";
        return res;
    }

    return "";
}

QString COMPortSender::req_tech()
{
    if(isActive(TECH))
    {
        QString res;
        QByteArray buffer(4, 0);
        buffer[0] = TECH_DEFAULT_ADDR;
        buffer[1] = 0x02;
        buffer[2] = 0x00;
        buffer[3] = 0x00;

        QByteArray readData1 = send(getPort(TECH), buffer);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if(z1 == 0)
            res += " Технол. модуль не готов к работе! \n";
        if(z2 == 1)
            res += " Ошибки у Технол. модуля! \n";
        if(z3 == 1)
            res += " Модуль Технол. после перезагрузки! \n";
        if(readData1.at(3)==0x10)
            res += " Потеря байта из-за переполнения буфера RS485! \n";
        return res;
    }

    return "";
}

int COMPortSender::softResetModule(ModuleID id)
{
    Q_ASSERT(id == STM || id == TECH);

    setActive(id, false);

    uint8_t moduleAddr = STM_DEFAULT_ADDR;
    if (id == TECH)
    {
        moduleAddr = TECH_DEFAULT_ADDR;
    }

    QByteArray buffer(4, 0);
    buffer[0] = moduleAddr;
    buffer[1] = SOFT_RESET;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QSerialPort * port = getPort(id);
    QByteArray readData1 = send(port, buffer);

    resetPort(id);
    setActive(id, true);

    return readData1[3];
}

void COMPortSender::resetPort(ModuleID id)
{
    ModuleInfo info = m_modules.take(id);

    info.port->close();
    info.port->deleteLater();
    info.port = Q_NULLPTR;

    for(int i = 0; i < 500; i++) // i guess some sort of govnomagics here "freeze app for 5 seconds to restore COM port with module after reset"
    {
        Sleep(10);
        QApplication::processEvents();
    }

    info.port = createPort("");
    m_modules[id] = info;
}

bool COMPortSender::isActive(ModuleID id) const
{
    return m_modules.value(id, ModuleInfo()).state;
}

void COMPortSender::setActive(ModuleID id, bool state)
{
    ModuleInfo info = m_modules.take(id);
    info.state = state;
    m_modules[id] = info;
}

int COMPortSender::getSoftwareVersion(ModuleID id)
{
    QSerialPort * port = getPort(id);
    Q_ASSERT(id == STM || id == TECH);

    uint8_t moduleAddr = STM_DEFAULT_ADDR;
    if (id == TECH)
    {
        moduleAddr = TECH_DEFAULT_ADDR;
    }

    QByteArray buffer(4, 0);
    buffer[0] = moduleAddr;
    buffer[1] = GET_SOWFTWARE_VER;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    QByteArray readData1 = send(port, buffer);
    return (readData1[2] * 10 + readData1[3]); // версия прошивки, ИМХО неправильно считается, т.к. два байта на нее
}


