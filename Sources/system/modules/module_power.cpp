#include "Headers/system/modules/module_power.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QVariant>

namespace
{
    static const uint32_t STEPS_COUNT = 25600; // hardware steps count to set/get voltage/current value
    static const int WAIT_FOR_RESPONSE_TIME = 100; // msec

    static const qreal MAX_ALLOWED_VOLTAGE = 36; // volts 27-36
    static const qreal MAX_ALLOWED_CURRENT = 2.0; // ampers <= 2
    static const qreal MIN_VOLTAGE = 27; // volts

    // nominal device parameters by default
    static const qreal DEFAULT_NOMINAL_POWER = 160; // watts
    static const qreal DEFAULT_NOMINAL_VOLTAGE = 42; // volts
    static const qreal DEFAULT_NOMINAL_CURRENT = 10; // ampers

    float byteArrayToFloat(const QByteArray& array)
    {
        if (array.size() != 4)
        {
            LOG_ERROR("Can not convert byte array to float, incorrect array size");
            return 0;
        }

        static_assert(std::numeric_limits<float>::is_iec559, "Only supports IEC559 (IEEE 754) float");

        qint32 tmp = (char(array[0]) << 24) | (char(array[1]) << 16) | (char(array[2]) << 8) | char(array[3]);
        float* value = reinterpret_cast<float*>(&tmp);
        return *value;
    }
}

ModulePower::ModulePower(QObject* parent):
    COMPortModule(parent),
    mState(ModuleCommands::POWER_OFF),
    mUpdatePeriod(0),
    mVoltageThreshold(MAX_ALLOWED_VOLTAGE),
    mCurrentThreshold(MAX_ALLOWED_CURRENT),
    mVoltage(0),
    mCurrent(0),
    mNominalVoltage(DEFAULT_NOMINAL_VOLTAGE),
    mNominalCurrent(DEFAULT_NOMINAL_CURRENT),
    mNominalPower(DEFAULT_NOMINAL_POWER),
    mDeviceClass(0),
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
    // - device class
    // - nominal voltage
    // - nominal current
    // - nominal power
    // - OVP threshold
    // - OCP threshold
    // - actual device state (voltage, current, alarms)

    mDeviceClass = getDeviceClass();
    mNominalCurrent = getNominalValue(NOMINAL_CURRENT);
    mNominalVoltage = getNominalValue(NOMINAL_VOLTAGE);
    mNominalPower = getNominalValue(NOMINAL_POWER);

    mVoltageThreshold = getObjectValue(OVP_THRESHOLD, mNominalVoltage);
    mCurrentThreshold = getObjectValue(OCP_THRESHOLD, mNominalCurrent);

    getCurVoltageAndCurrent(mVoltage, mCurrent, mError);

    // switch module to remote control
    sendPowerSupplyControlCommand(SWITCH_TO_REMOTE_CTRL);

    // set voltage and current thresholds
    setObjectValue(OVP_THRESHOLD, MAX_ALLOWED_VOLTAGE, mNominalVoltage);
    setObjectValue(OCP_THRESHOLD, MAX_ALLOWED_CURRENT, mNominalCurrent);

    // OPTIONAL (for logging only): //TODO
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
    if (!mIsInitialized)
    {
        LOG_ERROR("Module is not ready");
        return;
    }

    sendPowerSupplyControlCommand(ACKNOWLEDGE_ALARMS); // reset errors if it exist
    mError = 0;

    if (sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_OFF)) // switch off power output
    {
        mState = ModuleCommands::POWER_OFF;
    }

    setCurVoltage(MIN_VOLTAGE);

    // switch "give power supply" on
    if (sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_ON))
    {
        mState = ModuleCommands::POWER_ON;
    }
}

void ModulePower::setCurVoltage(qreal voltage)
{
    QMap<uint32_t, QVariant> request;
    QMap<uint32_t, QVariant> response;
    request[SystemState::OUTPUT_PARAM_BASE + 1] = voltage;
    setVoltageAndCurrent(request, response);
}

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
    return send(request, response, WAIT_FOR_RESPONSE_TIME);
}

qreal ModulePower::getObjectValue(ObjectID objectID, qreal nominalValue)
{
    switch (objectID)
    {
    case OVP_THRESHOLD:
    case OCP_THRESHOLD:
    case SET_VALUE_U:
    case SET_VALUE_I:
        {
            QByteArray request;
            request.append(encodeStartDelimiter(TO_DEVICE, SEND, QUERY_DATA, 2)); // 2 bytes of data is waiting in response
            request.append(SINGLE_MODEL);
            request.append(objectID);
            addCheckSum(request);

            QByteArray response;
            if (send(request, response, WAIT_FOR_RESPONSE_TIME))
            {
                uint8_t uu1, uu2;
                uu1 = response[3];
                uu2 = response[4];
                qreal value  = (uu1 << 8) | uu2;
                value = value * nominalValue / qreal(STEPS_COUNT);
                return value;
            }
        }
        break;

    default:
        LOG_WARNING("Incorrect object id '%i'", objectID);
        break;
    }

    return 0;
}

void ModulePower::getCurVoltageAndCurrent(qreal& voltage, qreal& current, uint8_t& error)
{
    QByteArray request;
    request.append(encodeStartDelimiter(TO_DEVICE, SEND, QUERY_DATA, 6)); // 6 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(DEVICE_STATUS_ACTUAL);
    addCheckSum(request);

    QByteArray response;
    if (send(request, response, WAIT_FOR_RESPONSE_TIME))
    {
        uint8_t uu1, uu2;
        error = (response[4] >> 4);
        int TODO; // parse error and save it locally

        uu1 = response[5];
        uu2 = response[6];
        voltage = (uu1 << 8) | uu2;
        voltage = voltage * mNominalVoltage / STEPS_COUNT;

        uu1 = response[7];
        uu2 = response[8];
        current = (uu1 << 8) | uu2;
        current = current * mNominalCurrent / STEPS_COUNT;
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

    int TODO; // do not process command in not initialized state

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

//    case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:
//        setMaxVoltageAndCurrent(params, response);
//        break;

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
    //uint32_t paramType = request.value(SystemState::INPUT_PARAM_BASE + 0).toUInt();
    qreal voltage      = request.value(SystemState::INPUT_PARAM_BASE + 1).toDouble();
    //uint32_t paramType2 = request.value(SystemState::INPUT_PARAM_BASE + 2).toUInt();
    //qreal value2        = request.value(SystemState::INPUT_PARAM_BASE + 3).toDouble();

    //qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
    //qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

    LOG_INFO("Try to set current voltage to : U=%f", voltage);

    if (MIN_VOLTAGE > mVoltageThreshold || MIN_VOLTAGE > mNominalVoltage)
    {
        LOG_FATAL("Unsupported voltage values: Umin=%f, OVPThr=%f, Unom=%f", MIN_VOLTAGE, mVoltageThreshold, mNominalVoltage);
        return;
    }

    // Set voltage value
    // It must be:
    // 1. Less than nominal power supply device voltage
    // 2. Less than max allowed by test stand voltage
    // 3. Greater than minimum required by test stand voltage
    qreal voltageToSet = voltage;
    if (voltageToSet > mNominalVoltage)
    {
        LOG_WARNING("Set voltage value has limited by nominal power supply unit voltage from %f to %f volts", voltage, mNominalVoltage);
        voltageToSet = mNominalVoltage;
    }

    if (voltageToSet > mVoltageThreshold)
    {
        LOG_WARNING("Set voltage value has limited by max allowed test stand voltage from %f to %f volts", voltageToSet, mVoltageThreshold);
        voltageToSet = mVoltageThreshold;
    }

    if (voltageToSet < MIN_VOLTAGE)
    {
        LOG_WARNING("Set voltage value has limited by min allowed test stand voltage from %f to %f volts", voltageToSet, voltageToSet);
        voltageToSet = voltageToSet;
    }

    setObjectValue(SET_VALUE_U, voltageToSet, mNominalVoltage);

    // Set current value
    // We always set maximum allowed current by power supply unit nominal power (take 95% of it for safety purposes)
    qreal maxCurrentByPower = qMin(mNominalPower * 0.95 / voltageToSet, mNominalCurrent);
    setObjectValue(SET_VALUE_I, maxCurrentByPower, mNominalCurrent);

    LOG_INFO("Actually set power supply params: U=%f I=%f", voltageToSet, maxCurrentByPower);

    // fill response
    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
}

//void ModulePower::setMaxVoltageAndCurrent(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
//{
//    // get input params
//    uint32_t paramType1 = request.value(SystemState::INPUT_PARAM_BASE + 0).toUInt();
//    qreal value1        = request.value(SystemState::INPUT_PARAM_BASE + 1).toDouble();
//    uint32_t paramType2 = request.value(SystemState::INPUT_PARAM_BASE + 2).toUInt();
//    qreal value2        = request.value(SystemState::INPUT_PARAM_BASE + 3).toDouble();

//    qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
//    qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

//    // execute command
//    setValue(OVP_THRESHOLD, voltage, mNominalVoltage);
//    setValue(OCP_THRESHOLD, current, mNominalCurrent);
//    qreal uc = qMin(voltage, mNominalVoltage);
//    qreal ic = qMin(current, mNominalCurrent);

//    int TODO; // possibly need to limit current values to new max values

//    LOG_INFO("setMaxVoltageAndCurrent: try to set values U=%f I=%f", voltage, current);
//    LOG_INFO("setMaxVoltageAndCurrent: actually set values U=%f I=%f", uc, ic);

//    // fill response
//    response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
//}

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
    return send(request, response, WAIT_FOR_RESPONSE_TIME);
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

qreal ModulePower::getNominalValue(ObjectID objectID)
{
    switch (objectID)
    {
    case NOMINAL_VOLTAGE:
    case NOMINAL_CURRENT:
    case NOMINAL_POWER:
        {
            QByteArray request;
            request.append(encodeStartDelimiter(TO_DEVICE, SEND, QUERY_DATA, 4));
            request.append(SINGLE_MODEL);
            request.append(objectID);
            addCheckSum(request);

            QByteArray response;
            if (send(request, response, WAIT_FOR_RESPONSE_TIME))
            {
                QByteArray tmp;
                tmp.push_back(response[3]);
                tmp.push_back(response[4]);
                tmp.push_back(response[5]);
                tmp.push_back(response[6]);
                return qreal(byteArrayToFloat(tmp));
            }
        }
        break;

    default:
        LOG_WARNING("Incorrect object id '%i'", objectID);
        break;
    }

    return 0;
}

uint16_t ModulePower::getDeviceClass()
{
    uint16_t value = 0;

    QByteArray request;
    request.append(encodeStartDelimiter(TO_DEVICE, SEND, QUERY_DATA, 2)); // 2 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(DEVICE_CLASS);
    addCheckSum(request);

    QByteArray response;
    if (send(request, response, WAIT_FOR_RESPONSE_TIME))
    {
        uint8_t uu1, uu2;
        uu1 = response[3];
        uu2 = response[4];
        value  = (uu1 << 8) | uu2;
    }

    return value;
}

void ModulePower::onApplicationFinish()
{
    int TODO; // power off on app close
}
