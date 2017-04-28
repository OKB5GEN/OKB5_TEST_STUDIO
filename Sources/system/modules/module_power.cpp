#include "Headers/system/modules/module_power.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QMetaEnum>

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
            LOG_ERROR(QString("Can not convert byte array to float, incorrect array size"));
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

void ModulePower::setObjectValue(ObjectID objectID, qreal actualValue, qreal nominalValue)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[OVP_THRESHOLD] = ModuleCommands::SET_OVP_THRESHOLD;
        operations[OCP_THRESHOLD] = ModuleCommands::SET_OCP_THRESHOLD;
        operations[SET_VALUE_U] = ModuleCommands::SET_SET_VALUE_U;
        operations[SET_VALUE_I] = ModuleCommands::SET_SET_VALUE_I;
    }

    ModuleCommands::CommandID operation = ModuleCommands::CommandID(operations.value(objectID, ModuleCommands::UNDEFINED));
    if (operation == ModuleCommands::UNDEFINED)
    {
        LOG_ERROR(QString("Incorrect object id '%1'").arg(objectID));
        return;
    }

    QByteArray request;
    uint16_t internalValue = uint16_t(qreal(STEPS_COUNT) * actualValue / nominalValue);

    request.append(encodeStartDelimiter(SEND_DATA, 2));
    request.append(SINGLE_MODEL);
    request.append(objectID);
    request.append((internalValue >> 8) & 0x00ff);
    request.append(internalValue & 0x00ff);
    addCheckSum(request);

    addRequest(operation, request);
}

void ModulePower::getObjectValue(ObjectID objectID)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[OVP_THRESHOLD] = ModuleCommands::GET_OVP_THRESHOLD;
        operations[OCP_THRESHOLD] = ModuleCommands::GET_OCP_THRESHOLD;
    }

    ModuleCommands::CommandID operation = ModuleCommands::CommandID(operations.value(objectID, ModuleCommands::UNDEFINED));
    if (operation == ModuleCommands::UNDEFINED)
    {
        LOG_ERROR(QString("Incorrect object id '%1'").arg(objectID));
        return;
    }

    QByteArray request;
    request.append(encodeStartDelimiter(QUERY_DATA, 2)); // 2 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(objectID);
    addCheckSum(request);

    addRequest(operation, request);
}

void ModulePower::getCurVoltageAndCurrent()
{
    QByteArray request;
    request.append(encodeStartDelimiter(QUERY_DATA, 6)); // 6 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(DEVICE_STATUS_ACTUAL);
    addCheckSum(request);

    addRequest(ModuleCommands::GET_VOLTAGE_AND_CURRENT, request);
}

void ModulePower::processCommand(const Transaction& params)
{
    mCurrentTransaction.clear();
    mCurrentTransaction = params;

    ModuleCommands::CommandID command = ModuleCommands::CommandID(params.commandID);

    switch (command)
    {
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        getCurVoltageAndCurrent();
        break;

    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        {
            qreal voltage = params.inputParams.value(SystemState::VOLTAGE).toDouble();
            setVoltageAndCurrent(voltage);
        }
        break;

    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
        sendPowerSupplyControlCommand(ACKNOWLEDGE_ALARMS);
        break;
    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
        sendPowerSupplyControlCommand(SWITCH_TO_REMOTE_CTRL);
        break;
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
        sendPowerSupplyControlCommand(SWITCH_TO_MANUAL_CTRL);
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
        sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_ON);
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
        sendPowerSupplyControlCommand(SWITCH_POWER_OUTPUT_OFF);
        break;
    case ModuleCommands::PSC_TRACKING_ON:
        sendPowerSupplyControlCommand(TRACKING_ON);
        break;
    case ModuleCommands::PSC_TRACKING_OFF:
        sendPowerSupplyControlCommand(TRACKING_OFF);
        break;
    case ModuleCommands::SET_OVP_THRESHOLD:
        setObjectValue(OVP_THRESHOLD, MAX_ALLOWED_VOLTAGE, mNominalVoltage);
        break;
    case ModuleCommands::SET_OCP_THRESHOLD:
        setObjectValue(OCP_THRESHOLD, MAX_ALLOWED_CURRENT, mNominalCurrent);
        break;
    case ModuleCommands::GET_DEVICE_CLASS:
        getDeviceClass();
        break;
    case ModuleCommands::GET_NOMINAL_CURRENT:
        getNominalValue(NOMINAL_CURRENT);
        break;
    case ModuleCommands::GET_NOMINAL_VOLTAGE:
        getNominalValue(NOMINAL_VOLTAGE);
        break;
    case ModuleCommands::GET_NOMINAL_POWER:
        getNominalValue(NOMINAL_POWER);
        break;
    case ModuleCommands::GET_OVP_THRESHOLD:
        getObjectValue(OVP_THRESHOLD);
        break;
    case ModuleCommands::GET_OCP_THRESHOLD:
        getObjectValue(OCP_THRESHOLD);
        break;

    default:
        {
            mCurrentTransaction.error = QString("Unknown command id=%1").arg(int(command));
            emit commandResult(mCurrentTransaction);
            return;
        }
        break;
    }
}

void ModulePower::setVoltageAndCurrent(qreal voltage)
{
    LOG_INFO(QString("Try to set current voltage to : U=%1").arg(voltage));

    if (MIN_VOLTAGE > mVoltageThreshold || MIN_VOLTAGE > mNominalVoltage)
    {
        LOG_FATAL(QString("Unsupported voltage values: Umin=%1, OVPThr=%2, Unom=%3").arg(MIN_VOLTAGE).arg(mVoltageThreshold).arg(mNominalVoltage));
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
        LOG_WARNING(QString("Set voltage value has limited by nominal power supply unit voltage from %1 to %2 volts").arg(voltage).arg(mNominalVoltage));
        voltageToSet = mNominalVoltage;
    }

    if (voltageToSet > mVoltageThreshold)
    {
        LOG_WARNING(QString("Set voltage value has limited by max allowed test stand voltage from %1 to %2 volts").arg(voltageToSet).arg(mVoltageThreshold));
        voltageToSet = mVoltageThreshold;
    }

    if (voltageToSet < MIN_VOLTAGE)
    {
        LOG_WARNING(QString("Set voltage value has limited by min allowed test stand voltage from %1 to %2 volts").arg(voltageToSet).arg(voltageToSet));
        voltageToSet = voltageToSet;
    }

    setObjectValue(SET_VALUE_U, voltageToSet, mNominalVoltage);

    // Set current value
    // We always set maximum allowed current by power supply unit nominal power (take 95% of it for safety purposes)
    qreal maxCurrentByPower = qMin(mNominalPower * 0.95 / voltageToSet, mNominalCurrent);
    setObjectValue(SET_VALUE_I, maxCurrentByPower, mNominalCurrent);

    LOG_INFO(QString("Actually set power supply params: U=%1 I=%2").arg(voltageToSet).arg(maxCurrentByPower));
}

void ModulePower::sendPowerSupplyControlCommand(PowerSupplyCommandID command)
{
    static QMap<int, int> operations;
    if (operations.empty())
    {
        operations[SWITCH_POWER_OUTPUT_ON] = ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON;
        operations[SWITCH_POWER_OUTPUT_OFF] = ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF;
        operations[ACKNOWLEDGE_ALARMS] = ModuleCommands::PSC_ACKNOWLEDGE_ALARMS;
        operations[SWITCH_TO_REMOTE_CTRL] = ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL;
        operations[SWITCH_TO_MANUAL_CTRL] = ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL;
        operations[TRACKING_ON] = ModuleCommands::PSC_TRACKING_ON;
        operations[TRACKING_OFF] = ModuleCommands::PSC_TRACKING_OFF;
    }

    ModuleCommands::CommandID operation = ModuleCommands::CommandID(operations.value(command, ModuleCommands::UNDEFINED));
    if (operation == ModuleCommands::UNDEFINED)
    {
        LOG_ERROR(QString("Incorrect command id '%1'").arg(command));
        return;
    }

    QByteArray request;
    request.append(encodeStartDelimiter(SEND_DATA, 2));
    request.append(SINGLE_MODEL);
    request.append(POWER_SUPPLY_CONTROL);
    uint16_t cmd = command;
    request.append(uint8_t((cmd >> 8) & 0x00ff));
    request.append(uint8_t(cmd & 0x00ff));
    addCheckSum(request);

    addRequest(operation, request);
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
        operations[NOMINAL_VOLTAGE] = ModuleCommands::GET_NOMINAL_VOLTAGE;
        operations[NOMINAL_CURRENT] = ModuleCommands::GET_NOMINAL_CURRENT;
        operations[NOMINAL_POWER] = ModuleCommands::GET_NOMINAL_POWER;
    }

    ModuleCommands::CommandID operation = ModuleCommands::CommandID(operations.value(objectID, ModuleCommands::UNDEFINED));
    if (operation == ModuleCommands::UNDEFINED)
    {
        LOG_ERROR(QString("Incorrect object id '%1'").arg(objectID));
        return;
    }

    QByteArray request;
    request.append(encodeStartDelimiter(QUERY_DATA, 4));
    request.append(SINGLE_MODEL);
    request.append(objectID);
    addCheckSum(request);

    addRequest(operation, request);
}

void ModulePower::getDeviceClass()
{
    QByteArray request;

    request.append(encodeStartDelimiter(QUERY_DATA, 2)); // 2 bytes of data is waiting in response
    request.append(SINGLE_MODEL);
    request.append(DEVICE_CLASS);
    addCheckSum(request);

    addRequest(ModuleCommands::GET_DEVICE_CLASS, request);
}

void ModulePower::addResponseParam(uint32_t paramID, QVariant value)
{
    QString var = mCurrentTransaction.outputParams.value(paramID).toString();

    if (var.isEmpty())
    {
        LOG_ERROR(QString("No output variable found for paramID=%1").arg(paramID));
        return;
    }

    QList<QVariant> list;
    list.append(QVariant(var));
    list.append(value);
    mCurrentTransaction.outputParams[paramID] = list;
}

bool ModulePower::processResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    QMetaEnum e = QMetaEnum::fromType<ModuleCommands::CommandID>();

    LOG_DEBUG(QString("%1: operation %2 response received").arg(moduleName()).arg(e.valueToKey(operationID)));

    switch (operationID)
    {
    case ModuleCommands::GET_DEVICE_CLASS:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mDeviceClass = (uu1 << 8) | uu2;
            addResponseParam(SystemState::DEVICE_CLASS, mDeviceClass);
        }
        break;

    case ModuleCommands::GET_NOMINAL_CURRENT:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalCurrent = qreal(byteArrayToFloat(tmp));
            addResponseParam(SystemState::CURRENT, mNominalCurrent);
        }
        break;

    case ModuleCommands::GET_NOMINAL_VOLTAGE:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalVoltage = qreal(byteArrayToFloat(tmp));
            addResponseParam(SystemState::VOLTAGE, mNominalVoltage);
        }
        break;

    case ModuleCommands::GET_NOMINAL_POWER:
        {
            QByteArray tmp;
            tmp.push_back(response[3]);
            tmp.push_back(response[4]);
            tmp.push_back(response[5]);
            tmp.push_back(response[6]);
            mNominalPower = qreal(byteArrayToFloat(tmp));
            addResponseParam(SystemState::POWER, mNominalPower);
        }
        break;

    case ModuleCommands::SET_OVP_THRESHOLD:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case ModuleCommands::SET_OCP_THRESHOLD:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case ModuleCommands::SET_SET_VALUE_I:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case ModuleCommands::SET_SET_VALUE_U:
        {
            int TODO; // parse response that is all OK
        }
        break;

    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        {
            uint8_t uu1, uu2;
            mError = (response[4] >> 4);
            QString error;

            switch (mError)
            {
            case 0: // no error
                break;
            case 1:
                error = QString("Overvoltage protection!");
                break;
            case 2:
                error = QString("Overcurrent protection!");
                break;
            case 4:
                error = QString("Overpower protection!");
                break;
            case 8:
                error = QString("Overtemperature protection!");
                break;

            default:
                error = QString("Unknown error: %1").arg(mError);
                break;
            }

            uu1 = response[5];
            uu2 = response[6];
            mVoltage = (uu1 << 8) | uu2;
            mVoltage = mVoltage * mNominalVoltage / STEPS_COUNT;

            uu1 = response[7];
            uu2 = response[8];
            mCurrent = (uu1 << 8) | uu2;
            mCurrent = mCurrent * mNominalCurrent / STEPS_COUNT;

            mCurrentTransaction.error = error;

            addResponseParam(SystemState::VOLTAGE, mVoltage);
            addResponseParam(SystemState::CURRENT, mCurrent);
        }
        break;

    case ModuleCommands::GET_OVP_THRESHOLD:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mVoltageThreshold  = (uu1 << 8) | uu2;
            mVoltageThreshold = mVoltageThreshold * mNominalVoltage / qreal(STEPS_COUNT);
            addResponseParam(SystemState::VOLTAGE, mVoltageThreshold);
        }
        break;

    case ModuleCommands::GET_OCP_THRESHOLD:
        {
            uint8_t uu1, uu2;
            uu1 = response[3];
            uu2 = response[4];
            mCurrentThreshold  = (uu1 << 8) | uu2;
            mCurrentThreshold = mCurrentThreshold * mNominalCurrent / qreal(STEPS_COUNT);
            addResponseParam(SystemState::CURRENT, mCurrentThreshold);
        }
        break;

    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
    case ModuleCommands::PSC_TRACKING_ON:
    case ModuleCommands::PSC_TRACKING_OFF:
        {
            int TODO; // parse response that is all OK
        }
        break;
    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
        {
            int TODO; // parse response, to not have errors
            mError = 0;
        }
        break;

    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
        {
            int TODO; // parse response, to not have errors
            mState = ModuleCommands::POWER_OFF;
        }
        break;

    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
        {
            int TODO; // parse response, to not have errors
            mState = ModuleCommands::POWER_ON;
        }
        break;

    default:
        LOG_WARNING(QString("Unexpected response received. Operation is %1").arg(int(operationID)));
        return false;
        break;
    }

    return true;
}

void ModulePower::onTransmissionError(uint32_t operationID)
{
    QMetaEnum e = QMetaEnum::fromType<ModuleCommands::CommandID>();
    mCurrentTransaction.error = QString("Operation %1 failed due to transmission error").arg(e.valueToKey(operationID));
    emit commandResult(mCurrentTransaction);
}

void ModulePower::onTransmissionComplete()
{
    if (!mCurrentTransaction.outputParams.isEmpty())
    {
        emit commandResult(mCurrentTransaction);
    }
}

void ModulePower::onApplicationFinish()
{
    int TODO; // check state and write errors/warnings

    closePort();
}
