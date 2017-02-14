#include "Headers/system/modules/module_power.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QVariant>

namespace
{
    static const uint32_t STEPS_COUNT = 25600; // hardware steps count to set/get voltage/current value

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
}

ModulePower::~ModulePower()
{

}

void ModulePower::initializeCustom()
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

    // perform first step: get nominal values
    getDeviceClass();
    getNominalValue(NOMINAL_CURRENT);
    getNominalValue(NOMINAL_VOLTAGE);
    getNominalValue(NOMINAL_POWER);

    processQueue();

    // OPTIONAL (for logging only): //TODO
    // - device type
    // - device serial number
    // - device article number
    // - device software version
    // - device manufacturer
}

void ModulePower::resetError()
{
    sendPowerSupplyControlCommand(ACKNOWLEDGE_ALARMS);
}

void ModulePower::restart()
{
    //if (mError != 0)
    {
        resetError();
    }

    sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_OFF); // switch off power output
    setCurVoltage(MIN_VOLTAGE);

    // switch "give power supply" on
    sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_ON);
    processQueue();
}

void ModulePower::setCurVoltage(qreal voltage)
{
    QMap<uint32_t, QVariant> request;
    request[SystemState::INPUT_PARAM_BASE + 1] = voltage;
    setVoltageAndCurrent(request);
}

void ModulePower::setObjectValue(ObjectID objectID, qreal actualValue, qreal nominalValue)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[OVP_THRESHOLD] = SET_OVP_THRESHOLD;
        operations[OCP_THRESHOLD] = SET_OCP_THRESHOLD;
        operations[SET_VALUE_U] = SET_SET_VALUE_U;
        operations[SET_VALUE_I] = SET_SET_VALUE_I;
    }

    Operation operation = Operation(operations.value(objectID, UNKNOWN_OPERATION));
    if (operation == UNKNOWN_OPERATION)
    {
        LOG_ERROR("Incorrect object id '%i'", objectID);
        return;
    }

    Request request;
    uint16_t internalValue = uint16_t(qreal(STEPS_COUNT) * actualValue / nominalValue);

    request.data.append(encodeStartDelimiter(SEND_DATA, 2));
    request.data.append(SINGLE_MODEL);
    request.data.append(objectID);
    request.data.append((internalValue >> 8) & 0x00ff);
    request.data.append(internalValue & 0x00ff);
    addCheckSum(request.data);

    request.operation = operation;
    mRequestQueue.push_back(request);
}

void ModulePower::getObjectValue(ObjectID objectID)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[OVP_THRESHOLD] = GET_OVP_THRESHOLD;
        operations[OCP_THRESHOLD] = GET_OCP_THRESHOLD;
    }

    Operation operation = Operation(operations.value(objectID, UNKNOWN_OPERATION));
    if (operation == UNKNOWN_OPERATION)
    {
        LOG_ERROR("Incorrect object id '%i'", objectID);
        return;
    }

    Request request;
    request.data.append(encodeStartDelimiter(QUERY_DATA, 2)); // 2 bytes of data is waiting in response
    request.data.append(SINGLE_MODEL);
    request.data.append(objectID);
    addCheckSum(request.data);

    request.operation = operation;
    mRequestQueue.push_back(request);
}

void ModulePower::getCurVoltageAndCurrent()
{
    Request request;
    request.data.append(encodeStartDelimiter(QUERY_DATA, 6)); // 6 bytes of data is waiting in response
    request.data.append(SINGLE_MODEL);
    request.data.append(DEVICE_STATUS_ACTUAL);
    addCheckSum(request.data);

    request.operation = GET_CUR_VOLTAGE_AND_CURRENT;
    mRequestQueue.push_back(request);
}

void ModulePower::processCommand(const QMap<uint32_t, QVariant>& params)
{
    QMap<uint32_t, QVariant> response;

//    if (!mIsInitialized)
//    {
//        LOG_ERROR("Power module is not initialized");
//        response[SystemState::ERROR_CODE] = QVariant(uint32_t(200)); //TODO define error codes internal or hardware
//        emit commandResult(response);
//        return;
//    }

    ModuleCommands::CommandID command = ModuleCommands::CommandID(params.value(SystemState::COMMAND_ID).toUInt());

    switch (command)
    {
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        getVoltageAndCurrent(params);
        break;

    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        setVoltageAndCurrent(params);
        break;

//    case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:
//        setMaxVoltageAndCurrent(params);
//        break;

    case ModuleCommands::RESET_ERROR:
        resetError();
        break;

    case ModuleCommands::SET_POWER_STATE:
        setPowerState(params);
        break;

    default:
        {
            LOG_ERROR(QString("Unknown command id=%1").arg(int(command)));
            response[SystemState::ERROR_CODE] = QVariant(uint32_t(100)); //TODO define error codes internal or hardware
            emit commandResult(response);
            return;
        }
        break;
    }

    mRequestQueue.back().response[SystemState::MODULE_ID] = params.value(SystemState::MODULE_ID);
    mRequestQueue.back().response[SystemState::COMMAND_ID] = QVariant(uint32_t(command));
    mRequestQueue.back().response[SystemState::ERROR_CODE] = QVariant(uint32_t(0));
    mRequestQueue.back().response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);

    processQueue();
}

void ModulePower::getVoltageAndCurrent(const QMap<uint32_t, QVariant>& request)
{
    getCurVoltageAndCurrent(); // add request to queue

    uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
    QString varName1    = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toString();
    uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
    QString varName2    = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toString();

    if (paramType1 == SystemState::VOLTAGE)
    {
        mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + 0] = QVariant(varName1);
        mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + 2] = QVariant(varName2);
    }
    else
    {
        mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + 0] = QVariant(varName2);
        mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + 2] = QVariant(varName1);
    }
}

void ModulePower::setVoltageAndCurrent(const QMap<uint32_t, QVariant>& request)
{
    // get input params
    qreal voltage = request.value(SystemState::INPUT_PARAM_BASE + 1).toDouble();
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

void ModulePower::setPowerState(const QMap<uint32_t, QVariant>& request)
{
    int TODO;
}

void ModulePower::sendPowerSupplyControlCommand(PowerSupplyCommandID command)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[SWITCH_POWER_OUTPUT_ON] = PSC_SWITCH_POWER_OUTPUT_ON;
        operations[SWITCH_POWER_OUTPUT_OFF] = PSC_SWITCH_POWER_OUTPUT_OFF;
        operations[ACKNOWLEDGE_ALARMS] = PSC_ACKNOWLEDGE_ALARMS;
        operations[SWITCH_TO_REMOTE_CTRL] = PSC_SWITCH_TO_REMOTE_CTRL;
        operations[SWITCH_TO_MANUAL_CTRL] = PSC_SWITCH_TO_MANUAL_CTRL;
        operations[TRACKING_ON] = PSC_TRACKING_ON;
        operations[TRACKING_OFF] = PSC_TRACKING_OFF;
    }

    Operation operation = Operation(operations.value(command, UNKNOWN_OPERATION));
    if (operation == UNKNOWN_OPERATION)
    {
        LOG_ERROR("Incorrect command id '%i'", command);
        return;
    }

    Request request;
    request.data.append(encodeStartDelimiter(SEND_DATA, 2));
    request.data.append(SINGLE_MODEL);
    request.data.append(POWER_SUPPLY_CONTROL);

    uint16_t cmd = command;
    request.data.append(uint8_t((cmd >> 8) & 0x00ff));
    request.data.append(uint8_t(cmd & 0x00ff));

    addCheckSum(request.data);

    request.operation = operation;
    mRequestQueue.push_back(request);
}

uint8_t ModulePower::encodeStartDelimiter(TransmissionType trType, uint8_t dataSize)
{
    Direction dir = TO_DEVICE;
    CastType cType = SEND;

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

void ModulePower::getNominalValue(ObjectID objectID)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[NOMINAL_VOLTAGE] = GET_NOMINAL_VOLTAGE;
        operations[NOMINAL_CURRENT] = GET_NOMINAL_CURRENT;
        operations[NOMINAL_POWER] = GET_NOMINAL_POWER;
    }

    Operation operation = Operation(operations.value(objectID, UNKNOWN_OPERATION));
    if (operation == UNKNOWN_OPERATION)
    {
        LOG_ERROR("Incorrect object id '%i'", objectID);
        return;
    }

    Request request;
    request.data.append(encodeStartDelimiter(QUERY_DATA, 4));
    request.data.append(SINGLE_MODEL);
    request.data.append(objectID);
    addCheckSum(request.data);

    request.operation = operation;
    mRequestQueue.push_back(request);
}

void ModulePower::getDeviceClass()
{
    Request request;

    request.data.append(encodeStartDelimiter(QUERY_DATA, 2)); // 2 bytes of data is waiting in response
    request.data.append(SINGLE_MODEL);
    request.data.append(DEVICE_CLASS);
    addCheckSum(request.data);

    request.operation = GET_DEVICE_CLASS;
    mRequestQueue.push_back(request);
}

void ModulePower::onApplicationFinish()
{
    int TODO; // power off on app close
}

void ModulePower::processResponse(const QByteArray& response)
{
    if (response.isEmpty())
    {
        LOG_ERROR(QString("No response received by power module! Flushing request queue..."));
        mRequestQueue.clear();
        return;
    }

    Operation operation = Operation(mRequestQueue.front().operation);

    switch (operation)
    {
    case GET_DEVICE_CLASS:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mDeviceClass = (uu1 << 8) | uu2;
        }
        break;

    case GET_NOMINAL_CURRENT:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalCurrent = qreal(byteArrayToFloat(tmp));
        }
        break;

    case GET_NOMINAL_VOLTAGE:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalVoltage = qreal(byteArrayToFloat(tmp));
        }
        break;

    case GET_NOMINAL_POWER:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalPower = qreal(byteArrayToFloat(tmp));

            // second step of initialization, add some requests (TODO move to the initialization cyclogram?)
            getCurVoltageAndCurrent();
            sendPowerSupplyControlCommand(SWITCH_TO_REMOTE_CTRL); // switch module to remote control to have ability to set U and I

            getObjectValue(OVP_THRESHOLD);
            getObjectValue(OCP_THRESHOLD);

            // set voltage and current thresholds
            setObjectValue(OVP_THRESHOLD, MAX_ALLOWED_VOLTAGE, mNominalVoltage);
            setObjectValue(OCP_THRESHOLD, MAX_ALLOWED_CURRENT, mNominalCurrent);
        }
        break;

    case SET_OVP_THRESHOLD:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case SET_OCP_THRESHOLD:
        {
            if (!mInitializationFinished)
            {
                mInitializationFinished = true;
                emit initializationFinished(QString(""));
            }

            int TODO; // parse response that is all OK
        }
        break;

    case SET_SET_VALUE_I:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case SET_SET_VALUE_U:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case GET_CUR_VOLTAGE_AND_CURRENT:
        {
            uint8_t uu1, uu2;
            mError = (response[4] >> 4);
            if (mError != 0)
            {
                LOG_ERROR("Error occured! TODO");
            }
            int TODO; // parse error and save it locally

            uu1 = response[5];
            uu2 = response[6];
            mVoltage = (uu1 << 8) | uu2;
            mVoltage = mVoltage * mNominalVoltage / STEPS_COUNT;

            uu1 = response[7];
            uu2 = response[8];
            mCurrent = (uu1 << 8) | uu2;
            mCurrent = mCurrent * mNominalCurrent / STEPS_COUNT;

            if (!mRequestQueue.front().response.empty())
            {
                mRequestQueue.front().response[SystemState::ERROR_CODE] = QVariant(uint32_t(mError));
                mRequestQueue.front().response[SystemState::OUTPUT_PARAM_BASE + 1] = QVariant(mVoltage);
                mRequestQueue.front().response[SystemState::OUTPUT_PARAM_BASE + 3] = QVariant(mCurrent);
                mRequestQueue.front().response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(4);
            }
        }
        break;

    case GET_OVP_THRESHOLD:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mVoltageThreshold  = (uu1 << 8) | uu2;
            mVoltageThreshold = mVoltageThreshold * mNominalVoltage / qreal(STEPS_COUNT);
        }
        break;

    case GET_OCP_THRESHOLD:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mCurrentThreshold  = (uu1 << 8) | uu2;
            mCurrentThreshold = mCurrentThreshold * mNominalCurrent / qreal(STEPS_COUNT);
        }
        break;

    case PSC_SWITCH_TO_REMOTE_CTRL:
    case PSC_SWITCH_TO_MANUAL_CTRL:
    case PSC_TRACKING_ON:
    case PSC_TRACKING_OFF:
        {
            int TODO; // parse response that is all OK
        }
        break;
    case PSC_ACKNOWLEDGE_ALARMS:
        {
            int TODO; // parse response, to not have errors
            mError = 0;
        }
        break;

    case PSC_SWITCH_POWER_OUTPUT_OFF:
        {
            int TODO; // parse response, to not have errors
            mState = ModuleCommands::POWER_OFF;
        }
        break;

    case PSC_SWITCH_POWER_OUTPUT_ON:
        {
            int TODO; // parse response, to not have errors
            mState = ModuleCommands::POWER_ON;
        }
        break;

    default:
        LOG_WARNING(QString("Unexpected response received. Operation is %1").arg(int(operation)));
        break;
    }

    if (!mRequestQueue.front().response.empty())
    {
        emit commandResult(mRequestQueue.front().response);
    }

    mRequestQueue.pop_front();
    processQueue();
}
