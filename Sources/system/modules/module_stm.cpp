#include "Headers/system/modules/module_stm.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMetaEnum>

namespace
{
    static const int STM_DEFAULT_ADDR = 0x22;
    static const qreal CHANNEL_CONNECTED_BORDER = 2.0;
    static const int MAX_CHANNELS_COUNT = 16;
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

    addModuleCmd(ModuleCommands::POWER_CHANNEL_CTRL, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0);

    /*
    if (channel >= 0 && channel < MAX_CHANNELS_COUNT)
    {
        mChannelStates[state];
    }
    else
    {
        LOG_ERROR("Invalid STM channel index %i", channel);
    }*/
}

void ModuleSTM::setMKOPowerChannelState(int channel, ModuleCommands::PowerState state)
{
    //int TODO; // valid channel values 1 to 6

    addModuleCmd(ModuleCommands::SET_MKO_PWR_CHANNEL_STATE, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0);
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
    addModuleCmd(ModuleCommands::GET_CHANNEL_TELEMETRY, channel, 0);

    int TODO;
    //uint8_t uu1, uu2;
    //uu1 = response[2];
    //uu2 = response[3];
    //qreal voltage = qreal((uu1 << 8) | uu2) / 10000;

    //LOG_INFO("Channel voltage is %f", voltage);

    //if (voltage >= CHANNEL_CONNECTED_BORDER)
    {
    //    return ModuleCommands::POWER_ON;
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

    addModuleCmd(ModuleCommands::GET_PWR_MODULE_FUSE_STATE, fuseIndex, 0);
//        LOG_ERROR("Can not check fuse %i state", fuseIndex);
//        return ModuleSTM::ERROR;

    //TODO
    int state = 0; //response[3];

    QMetaEnum e = QMetaEnum::fromType<ModuleSTM::FuseStates>();
    LOG_INFO("Fuse %i state %s", fuseIndex, e.valueToKey(ModuleSTM::FuseStates(state)));

    return ModuleSTM::FuseStates(state); //TODO undefined values
}

void ModuleSTM::processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    mTmpResponse.clear();
    mTmpResponse = response;
    mTmpResponse.detach();

    ModuleCommands::CommandID command = ModuleCommands::CommandID(request.value(SystemState::COMMAND_ID).toUInt());

    switch (command)
    {
    case ModuleCommands::POWER_CHANNEL_CTRL:
        {
            int paramsCount = request.value(SystemState::IMPLICIT_PARAMS_COUNT).toInt();
            if (paramsCount != 2)
            {
                LOG_ERROR("Malformed request for STM command");
                response[SystemState::ERROR_CODE] = QVariant(1);
                return;
            }

            int channel = request.value(SystemState::IMPLICIT_PARAM_BASE + 0).toInt();
            ModuleCommands::PowerState state = ModuleCommands::PowerState(request.value(SystemState::IMPLICIT_PARAM_BASE + 1).toInt());

            setPowerChannelState(channel, state);
            response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
        }
        break;

    default:
        LOG_ERROR("Unexpected command %i received by STM module", command);
        break;
    }

    // set variables
    int paramsCount = request.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
    for (int i = 0; i < paramsCount; ++i)
    {
        QString varName = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2 + 1).toString();
        mTmpResponse[SystemState::OUTPUT_PARAM_BASE + i * 2] = QVariant(varName);
    }

    if (paramsCount > 0)
    {
        mTmpResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);
    }
}

bool ModuleSTM::processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(operationID);

    switch (command)
    {
    case ModuleCommands::POWER_CHANNEL_CTRL:
        {
            int TODO;
        }
        break;

    case ModuleCommands::GET_PWR_MODULE_FUSE_STATE:
        {
            int TODO;
        }
        break;

    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        {
            int TODO;
        }
        break;

    case ModuleCommands::SET_MKO_PWR_CHANNEL_STATE:
        {
            int TODO;
        }
        break;

    case ModuleCommands::GET_POWER_MODULE_STATE:
        {
            int TODO;
        }
        break;
    case ModuleCommands::GET_MKO_MODULE_STATE:
        {
            int TODO;
        }
        break;

    default:
        return false;
        break;
    }

    return true;
}

void ModuleSTM::onApplicationFinish()
{
    int TODO;
}

void ModuleSTM::onModuleError()
{
    int TODO; //TODO here will be processing
}

void ModuleSTM::createResponse(QMap<uint32_t, QVariant>& response)
{
    // fill response
    int paramsCount = mTmpResponse.value(SystemState::OUTPUT_PARAMS_COUNT, 0).toInt();
    int valuesCount = 0; //mTemperatureData.size();

    if (paramsCount != valuesCount)
    {
        LOG_ERROR("Request output params count (%i) and values count (%i) mismatch", paramsCount, valuesCount);
        return;
    }

    for (int i = 0; i < paramsCount; ++i)
    {
        mTmpResponse[SystemState::OUTPUT_PARAM_BASE + i * 2 + 1] = QVariant(0/*mTemperatureData[i]*/);
    }

    if (paramsCount > 0)
    {
        mTmpResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount * 2);
    }

    response = mTmpResponse;
}

void ModuleSTM::initializeCustomOKBModule()
{
    //TODO stub here
    mModuleReady = true;
    mCustomInitializationFinished = true;
    emit initializationFinished(QString(""));
}

void ModuleSTM::setDefaultState()
{
    int TODO;

    setMKOPowerChannelState(1, ModuleCommands::POWER_ON); // enable MKO power supply (main?) TODO
    setMKOPowerChannelState(2, ModuleCommands::POWER_ON); // enable MKO power supply (reserve?) TODO
}
