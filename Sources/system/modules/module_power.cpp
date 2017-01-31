#include "Headers/system/modules/module_power.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QVariant>

namespace
{
    static const uint32_t MAX_VOLTAGE = 42; // volts
    static const uint32_t MAX_CURRENT = 10; // ampers
    static const uint32_t MAX_POWER = 155; // watts (actually 160, but for safety purposes reduced to 155)
    static const uint32_t MAX_STEPS_COUNT = 25600; // hardware steps count to set/get voltage/current value
}

ModulePower::ModulePower(QObject* parent):
    COMPortModule(parent),
    mState(ModuleCommands::POWER_OFF),
    mUpdatePeriod(0),
    mVoltage(0),
    mCurrent(0),
    mError(0)
{
    mUpdateTimer = new QTimer(this);
    connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

ModulePower::~ModulePower()
{

}

bool ModulePower::postInit()
{
    return true;
}

void ModulePower::resetError()
{
    QByteArray request(7, 0);

    request[0] = 0xf1;
    request[1] = 0x00;
    request[2] = 0x36;
    request[3] = 0x0a;
    request[4] = 0x0a;
    request[5] = 0x01;
    request[6] = 0x3b;

    QByteArray response;
    send(request, response);
}

void ModulePower::startPower()
{
    /*
    resetError(); // reset error if it exist

    // PowerON
    QByteArray request1(7, 0); // switch "remote mode" on
    request1[0] = 0xf1;
    request1[1] = 0x00;
    request1[2] = 0x36;
    request1[3] = 0x10;
    request1[4] = 0x10;
    request1[5] = 0x01;
    request1[6] = 0x47;

    QByteArray response1;
    send(request1, response1);

    // TODO setPowerState(ModuleCommands::POWER_OFF); // switch "give power supply" off instead of code below?

    QByteArray request2(7, 0); // switch "give power supply" off
    request2[0] = 0xf1;//power off
    request2[1] = 0x00;
    request2[2] = 0x36;
    request2[3] = 0x01;
    request2[4] = 0x00;
    request2[5] = 0x01;
    request2[6] = 0x28;

    QByteArray response2;
    send(request2, response2);

    setMaxVoltageAndCurrent(28, 1.5); // set voltage and current limitations
    setVoltageAndCurrent(27, 0.4); // set current value for voltage and current

    setPowerState(ModuleCommands::POWER_ON); // switch "give power supply" on
    */
}

void ModulePower::setPowerState(ModuleCommands::PowerState state)
{
    QByteArray request(7, 0);
    request[0] = 0xf1;//power on/off
    request[1] = 0x00;
    request[2] = 0x36;
    request[3] = 0x01;
    request[4] = 0x01;
    request[5] = 0x01;
    request[6] = (state == ModuleCommands::POWER_ON) ? 0x29 : 0x28;

    QByteArray response;
    if (send(request, response))
    {
        mState = state;
    }
}

void ModulePower::setValue(uint8_t valueID, qreal value, qreal maxValue)
{
    QByteArray request(7, 0);
    uint32_t val = qMin(uint32_t((value * MAX_STEPS_COUNT) / maxValue), MAX_STEPS_COUNT);

    request[0] = 0xf1;
    request[1] = 0x00;
    request[2] = valueID;
    request[3] = (val >> 8) & 0xFF;
    request[4] = val & 0xFF;
    uint16_t sum = 0;
    for(int i = 0; i < 5; i++)
    {
        uint8_t s = request[i];
        sum = (sum + s) & 0xFFFF;
    }

    request[5] = ((sum >> 8) & 0xFF);
    request[6] = (sum & 0xFF);

    QByteArray response;
    send(request, response);
}

void ModulePower::getCurVoltageAndCurrent(qreal& voltage, qreal& current, uint8_t& error)
{
    QByteArray request(5, 0);
    request[0] = 0x75;
    request[1] = 0x00;
    request[2] = 0x47;
    request[3] = 0x00;
    request[4] = 0xbc;

    QByteArray response;
    send(request, response);

    if (response.size() >= 9)
    {
        uint8_t uu1, uu2;
        error = (response[4] >> 4);

        uu1 = response[5];
        uu2 = response[6];
        voltage = (uu1 << 8) | uu2;
        voltage = voltage * MAX_VOLTAGE / MAX_STEPS_COUNT;

        uu1 = response[7];
        uu2 = response[8];
        current = (uu1 << 8) | uu2;
        current = current * MAX_CURRENT / MAX_STEPS_COUNT;
    }
}

void ModulePower::setUpdatePeriod(int msec, bool startTimer)
{
    mUpdatePeriod = msec;

    if (startTimer)
    {
        if (mUpdatePeriod > 0)
        {
            mUpdateTimer->setInterval(mUpdatePeriod);
            mUpdateTimer->start();
        }
        else
        {
            mUpdateTimer->stop();
        }
    }
}

void ModulePower::update()
{
    getCurVoltageAndCurrent(mVoltage, mCurrent, mError);
}

void ModulePower::processCommand(const QMap<uint32_t, QVariant>& params)
{
    QMap<uint32_t, QVariant> response;

    ModuleCommands::CommandID command = ModuleCommands::CommandID(params.value(SystemState::COMMAND_ID).toUInt());

    response[SystemState::MODULE_ID] = params.value(SystemState::MODULE_ID);
    response[SystemState::COMMAND_ID] = QVariant(uint32_t(command));
    response[SystemState::ERROR_CODE] = QVariant(uint32_t(0));
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);

    switch (command)
    {
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        getVoltageAndCurrent(params, response);
        break;

    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        setVoltageAndCurrent(params, response);
        break;

    case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:
        setMaxVoltageAndCurrent(params, response);
        break;

    case ModuleCommands::RESET_ERROR:
        resetError();
        break;

    case ModuleCommands::SET_POWER_STATE:
        setPowerState(params, response);
        break;

    default:
        break;
    }

    // send response
    emit commandResult(response);
}

void ModulePower::getVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    // execute command
    qreal voltage = -1;
    qreal current = -1;
    uint8_t error = 0xff;
    getCurVoltageAndCurrent(voltage, current, error);

    // fill response
    response[SystemState::ERROR_CODE] = QVariant(uint32_t(error));

    uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
    QString varName1    = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toString();
    uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
    QString varName2    = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toString();

    if (paramType1 == SystemState::VOLTAGE)
    {
        response[SystemState::OUTPUT_PARAM_BASE + 0] = QVariant(varName1);
        response[SystemState::OUTPUT_PARAM_BASE + 1] = QVariant(voltage);
        response[SystemState::OUTPUT_PARAM_BASE + 2] = QVariant(varName2);
        response[SystemState::OUTPUT_PARAM_BASE + 3] = QVariant(current);
    }
    else
    {
        response[SystemState::OUTPUT_PARAM_BASE + 0] = QVariant(varName2);
        response[SystemState::OUTPUT_PARAM_BASE + 1] = QVariant(voltage);
        response[SystemState::OUTPUT_PARAM_BASE + 2] = QVariant(varName1);
        response[SystemState::OUTPUT_PARAM_BASE + 3] = QVariant(current);
    }

    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(4);
}

void ModulePower::setVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    // get input params
    uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
    qreal value1        = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toDouble();
    uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
    qreal value2        = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toDouble();

    qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
    qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

    // execute command

    int TODO; // possibly need to limit values to set by currently set max values

    // set voltage first, limitated by hardware value
    setValue(CUR_VOLTAGE_VAL, voltage, MAX_VOLTAGE);
    // set current, limitated by max hardware power and voltage value that was set
    qreal uc = qMin(voltage, (qreal)MAX_VOLTAGE);
    qreal maxCurrent = qMin((qreal)MAX_POWER / uc, (qreal)MAX_CURRENT);
    // the result power must be less than max allowed
    setValue(CUR_CURRENT_VAL, current, maxCurrent);
    qreal ic = qMin(current, maxCurrent);

    LOG_INFO("setVoltageAndCurrent: try to set values U=%f I=%f", voltage, current);
    LOG_INFO("setVoltageAndCurrent: actually set values U=%f I=%f", uc, ic);

    // fill response
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
}

void ModulePower::setMaxVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    // get input params
    uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
    qreal value1        = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toDouble();
    uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
    qreal value2        = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toDouble();

    qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
    qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

    // execute command
    setValue(MAX_VOLTAGE_VAL, voltage, MAX_VOLTAGE);
    setValue(MAX_CURRENT_VAL, current, MAX_CURRENT);
    qreal uc = qMin(voltage, (qreal)MAX_VOLTAGE);
    qreal ic = qMin(current, (qreal)MAX_CURRENT);

    int TODO; // possibly need to limit current values to new max values

    LOG_INFO("setMaxVoltageAndCurrent: try to set values U=%f I=%f", voltage, current);
    LOG_INFO("setMaxVoltageAndCurrent: actually set values U=%f I=%f", uc, ic);

    // fill response
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
}

void ModulePower::setPowerState(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    int TODO;
}
