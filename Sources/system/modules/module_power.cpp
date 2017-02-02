#include "Headers/system/modules/module_power.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QVariant>

namespace
{
    static const uint32_t NOMINAL_VOLTAGE_VALUE = 42; // volts
    static const uint32_t NOMINAL_CURRENT_VALUE = 6; // ampers
    static const uint32_t NOMINAL_POWER_VALUE = 100; // watts (actually 160, but for safety purposes reduced to 155)
    static const uint32_t STEPS_COUNT = 25600; // hardware steps count to set/get voltage/current value

    static const qreal MAX_ALLOWED_VOLTAGE = 36; // volts
    static const qreal MAX_ALLOWED_CURRENT = 0.7; // ampers
    static const qreal NORMAL_VOLTAGE = 27; // volts
    static const qreal NORMAL_CURRENT = 0.5; // ampers
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
    // receive device parameters:

    // MANDATORY:
    // - nominal voltage
    // - nominal current
    // - nominal power
    // - device class
    // - OVP threshold
    // - OCP threshold
    // - actual device state (voltage, current, alarms)

    // OPTIONAL (for logging only):
    // - device type
    // - device serial number
    // - device article number
    // - device software version
    // - device manufacturer

    return true;
}

void ModulePower::resetError()
{
    sendPowerSupplyControlCommand(ACKNOWLEDGE_ALARMS);
}

void ModulePower::restart()
{
    int TODO;

    //sendPowerSupplyControlCommand(ACKNOWLEDGE_ALARMS); // reset error if it exist
    //sendPowerSupplyControlCommand(SWITCH_TO_MANUAL_CTRL); // possibly not necessary?
    //sendPowerSupplyControlCommand(SWITCH_TO_REMOTE_CTRL); // possibly need to be set once at app start?
    if (sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_OFF)) // switch off power output
    {
        mState = ModuleCommands::POWER_OFF;
    }

    // set voltage and current limitations
    setObjectValue(OVP_THRESHOLD, MAX_ALLOWED_VOLTAGE, NOMINAL_VOLTAGE_VALUE);
    setObjectValue(OCP_THRESHOLD, MAX_ALLOWED_CURRENT, NOMINAL_CURRENT_VALUE);

    // set current value for voltage and current
    setObjectValue(SET_VALUE_U, NORMAL_VOLTAGE, NOMINAL_VOLTAGE_VALUE);
    setObjectValue(SET_VALUE_I, NORMAL_CURRENT, NOMINAL_CURRENT_VALUE);

    // switch "give power supply" on
    if (sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_ON))
    {
        mState = ModuleCommands::POWER_ON;
    }
}
/*
void ModulePower::setValue(ObjectID objectID, qreal value, qreal maxValue)
{
    QByteArray request;
    uint32_t internalValue = qMin(uint32_t((value * STEPS_COUNT) / maxValue), STEPS_COUNT);

    request.append(encodeStartDelimiter(TO_DEVICE, SEND, SEND_DATA, 2);
    request.append(SINGLE_MODEL);
    request.append(objectID);
    request.append((internalValue >> 8) & 0x00ff);
    request.append(internalValue & 0x00ff);
    addCheckSum(request);

    QByteArray response;
    send(request, response);
}
*/
bool ModulePower::setObjectValue(ObjectID objectID, qreal actualValue, qreal nominalValue)
{
    QByteArray request;
    uint16_t internalValue = uint16_t(qreal(STEPS_COUNT) * actualValue / nominalValue);

    request.append(encodeStartDelimiter(TO_DEVICE, SEND, SEND_DATA, 2));
    request.append(SINGLE_MODEL);
    request.append(objectID);
    request.append((internalValue >> 8) & 0x00ff);
    request.append(internalValue & 0x00ff);
    addCheckSum(request);

    QByteArray response;
    return send(request, response);
}

void ModulePower::getCurVoltageAndCurrent(qreal& voltage, qreal& current, uint8_t& error)
{
    QByteArray request;
    request.append(encodeStartDelimiter(TO_DEVICE, SEND, QUERY_DATA, 6)); // 6 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(DEVICE_STATUS_ACTUAL);
    addCheckSum(request);

    QByteArray response;
    if (send(request, response))
    {
        uint8_t uu1, uu2;
        error = (response[4] >> 4);

        uu1 = response[5];
        uu2 = response[6];
        voltage = (uu1 << 8) | uu2;
        voltage = voltage * NOMINAL_VOLTAGE_VALUE / STEPS_COUNT;

        uu1 = response[7];
        uu2 = response[8];
        current = (uu1 << 8) | uu2;
        current = current * NOMINAL_CURRENT_VALUE / STEPS_COUNT;
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
    /*
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
    setValue(SET_VALUE_U, voltage, NOMINAL_VOLTAGE_VALUE);
    // set current, limitated by max hardware power and voltage value that was set
    qreal uc = qMin(voltage, (qreal)NOMINAL_VOLTAGE_VALUE);
    qreal maxCurrent = qMin((qreal)NOMINAL_POWER_VALUE / uc, (qreal)NOMINAL_CURRENT_VALUE);
    // the result power must be less than max allowed
    setValue(SET_VALUE_I, current, maxCurrent);
    qreal ic = qMin(current, maxCurrent);

    LOG_INFO("setVoltageAndCurrent: try to set values U=%f I=%f", voltage, current);
    LOG_INFO("setVoltageAndCurrent: actually set values U=%f I=%f", uc, ic);

    // fill response
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
    */
}

void ModulePower::setMaxVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    /*
    // get input params
    uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
    qreal value1        = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toDouble();
    uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
    qreal value2        = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toDouble();

    qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
    qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

    // execute command
    setValue(OVP_THRESHOLD, voltage, NOMINAL_VOLTAGE_VALUE);
    setValue(OCP_THRESHOLD, current, NOMINAL_CURRENT_VALUE);
    qreal uc = qMin(voltage, (qreal)NOMINAL_VOLTAGE_VALUE);
    qreal ic = qMin(current, (qreal)NOMINAL_CURRENT_VALUE);

    int TODO; // possibly need to limit current values to new max values

    LOG_INFO("setMaxVoltageAndCurrent: try to set values U=%f I=%f", voltage, current);
    LOG_INFO("setMaxVoltageAndCurrent: actually set values U=%f I=%f", uc, ic);

    // fill response
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
    */
}

void ModulePower::setPowerState(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    int TODO;
}

bool ModulePower::sendPowerSupplyControlCommand(PowerSupplyCommandID command)
{
    QByteArray request;
    request.append(encodeStartDelimiter(TO_DEVICE, SEND, SEND_DATA, 2));
    request.append(SINGLE_MODEL);
    request.append(POWER_SUPPLY_CONTROL);

    uint16_t cmd = command;
    request.append(uint8_t((cmd >> 8) & 0x00ff));
    request.append(uint8_t(cmd & 0x00ff));

    addCheckSum(request);

    QByteArray response;
    return send(request, response);
}

uint8_t ModulePower::encodeStartDelimiter(Direction dir, CastType cType, TransmissionType trType, uint8_t dataSize)
{
    uint8_t delimiter = 0;
    delimiter += uint8_t(dir);
    delimiter += uint8_t(cType);
    delimiter += uint8_t(trType);
    delimiter += ((dataSize - 1) & 0x0f);
    return delimiter;
}

void ModulePower::addCheckSum(QByteArray& data)
{
    uint16_t sum = 0;
    uint16_t sum2 = 0;
    int size = data.size();

    for(int i = 0; i < size; ++i)
    {
        uint8_t s = data[i];
        sum = (sum + s) & 0xffff;
        sum2 = sum2 + s;
    }

    data.append(uint8_t((sum >> 8) & 0x00ff));
    data.append(uint8_t(sum & 0x00ff));
}
