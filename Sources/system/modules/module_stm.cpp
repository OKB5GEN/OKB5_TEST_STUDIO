#include "Headers/system/modules/module_stm.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMetaEnum>

namespace
{
    static const int STM_DEFAULT_ADDR = 0x22;
    static const qreal CHANNEL_CONNECTED_BORDER = 2.0;
    static const int MAX_CHANNELS_COUNT = 16;
    static const int WAIT_FOR_RESPONSE_TIME = 100; // msec
}

ModuleSTM::ModuleSTM(QObject* parent):
    ModuleOKB(parent)
{
    mChannelStates.fill(ModuleCommands::POWER_OFF, MAX_CHANNELS_COUNT);
}

ModuleSTM::~ModuleSTM()
{

}

void ModuleSTM::setPowerChannelState(int channel, ModuleCommands::PowerState state)
{
    int TODO; // valid channel values 1 to 6

    if (!sendCommand(ModuleCommands::POWER_CHANNEL_CTRL, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0, WAIT_FOR_RESPONSE_TIME))
    {
        LOG_ERROR("Channel %i is not set to %i state", channel, state);
        return;
    }

    if (channel >= 0 && channel < MAX_CHANNELS_COUNT)
    {
        mChannelStates[state];
    }
    else
    {
        LOG_ERROR("Invalid STM channel index %i", channel);
    }
}

void ModuleSTM::setMKOPowerChannelState(int channel, ModuleCommands::PowerState state)
{
    //int TODO; // valid channel values 1 to 6

    if (!sendCommand(ModuleCommands::SET_MKO_PWR_CHANNEL_STATE, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0, WAIT_FOR_RESPONSE_TIME))
    {
        LOG_ERROR("MKO Channel %i is not set to %i state", channel, state);
        return;
    }

    /*if (channel >= 0 && channel < MAX_CHANNELS_COUNT)
    {
        mChannelStates[state];
    }
    else
    {
        LOG_ERROR("Invalid STM channel index %i", channel);
    }*/
}

ModuleCommands::PowerState ModuleSTM::powerChannelState(int channel)
{
    QByteArray response;

    if (!sendCommand(ModuleCommands::GET_CHANNEL_TELEMETRY, channel, 0, WAIT_FOR_RESPONSE_TIME, &response))
    {
        LOG_ERROR("Can not get channel %i telemetry", channel);
        return ModuleCommands::POWER_OFF;
    }

    uint8_t uu1, uu2;
    uu1 = response[2];
    uu2 = response[3];
    qreal voltage = qreal((uu1 << 8) | uu2) / 10000;

    LOG_INFO("Channel voltage is %f", voltage);

    if (voltage >= CHANNEL_CONNECTED_BORDER)
    {
        return ModuleCommands::POWER_ON;
    }

    return ModuleCommands::POWER_OFF;
}

ModuleSTM::FuseStates ModuleSTM::fuseState(int fuseIndex)
{
    // valid values 1 to 8 //TODO
    if (fuseIndex < 1 || fuseIndex > 8)
    {
        LOG_ERROR("Invalid fuse index %i", fuseIndex);
        return ModuleSTM::ERROR;
    }

    QByteArray response;
    if (!sendCommand(ModuleCommands::GET_PWR_MODULE_FUSE_STATE, fuseIndex, 0, WAIT_FOR_RESPONSE_TIME, &response))
    {
        LOG_ERROR("Can not check fuse %i state", fuseIndex);
        return ModuleSTM::ERROR;
    }

    int state = response[3];

    QMetaEnum e = QMetaEnum::fromType<ModuleSTM::FuseStates>();
    LOG_INFO("Fuse %i state %s", fuseIndex, e.valueToKey(ModuleSTM::FuseStates(state)));

    return ModuleSTM::FuseStates(state); //TODO undefined values
}

void ModuleSTM::processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(request.value(SystemState::COMMAND_ID).toUInt());

    switch (command)
    {
    case ModuleCommands::POWER_CHANNEL_CTRL:
        {
            int TODO;

            //setPowerChannelState(channel, state); // read from input params (they are implicit params)

            // get input params
            /*uint32_t paramType1 = request.value(SystemState::OUTPUT_PARAM_BASE + 0).toUInt();
            qreal value1        = request.value(SystemState::OUTPUT_PARAM_BASE + 1).toDouble();
            uint32_t paramType2 = request.value(SystemState::OUTPUT_PARAM_BASE + 2).toUInt();
            qreal value2        = request.value(SystemState::OUTPUT_PARAM_BASE + 3).toDouble();

            qreal voltage = (paramType1 == SystemState::VOLTAGE) ? value1 : value2;
            qreal current = (paramType1 == SystemState::VOLTAGE) ? value2 : value1;

            LOG_INFO("Try to set power supply params to: U=%f I=%f", voltage, current);

            // execute command
            qreal voltageToSet = limitValue(voltage, mNominalVoltage, mVoltageThreshold);
            qreal currentToSet = limitValue(current, mNominalCurrent, mCurrentThreshold);

            // set voltage
            setObjectValue(SET_VALUE_U, voltageToSet, mNominalVoltage);

            // limitated by max hardware power and voltage value that was set
            qreal maxCurrentByPower = qMin(mNominalPower / voltageToSet, mNominalCurrent);
            currentToSet = qMin(currentToSet, maxCurrentByPower);

            // set current
            setObjectValue(SET_VALUE_I, currentToSet, mNominalCurrent);

            LOG_INFO("Actually set power supply params: U=%f I=%f", voltageToSet, currentToSet);

            // fill response
            response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);*/
        }
        break;

    default:
        LOG_ERROR("Unexpected command %i received by STM module", command);
        break;
    }

    /*if (!temperature.empty())
    {
        // fill response
        int paramsCount = request.value(SystemState::OUTPUT_PARAMS_COUNT, 0).toInt();
        int valuesCount = temperature.size();

        if (paramsCount != valuesCount)
        {
            LOG_ERROR("Request output params count (%i) and values count (%i) mismatch", paramsCount, valuesCount);
            return;
        }

        for (int i = 0; i < paramsCount; ++i)
        {
            uint32_t paramType = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2).toUInt();
            QString varName    = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2 + 1).toString();

            response[SystemState::OUTPUT_PARAM_BASE + i * 2] = QVariant(varName);
            response[SystemState::OUTPUT_PARAM_BASE + i * 2 + 1] = QVariant(temperature[i]);
        }

        response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount * 2);
    }}*/
}
