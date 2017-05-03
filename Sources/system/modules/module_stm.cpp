#include "Headers/system/modules/module_stm.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

#include <QMetaEnum>

namespace
{
    static const int STM_DEFAULT_ADDR = 0x22;
    static const qreal CHANNEL_CONNECTED_BORDER = 2.0;
    static const int TELEMETRY_CHANNELS_COUNT = 16;
}

ModuleSTM::ModuleSTM(QObject* parent):
    ModuleOKB(parent),
    mRequestedChannelTelemetry(0)
{
//    mPowerSupplyRelayStates[ModuleCommands::BUP_MAIN] = ModuleCommands::POWER_OFF;
//    mPowerSupplyRelayStates[ModuleCommands::BUP_RESERVE] = ModuleCommands::POWER_OFF;
//    mPowerSupplyRelayStates[ModuleCommands::UNKNOWN_3] = ModuleCommands::POWER_OFF;
//    mPowerSupplyRelayStates[ModuleCommands::HEATER_LINE_1] = ModuleCommands::POWER_OFF;
//    mPowerSupplyRelayStates[ModuleCommands::HEATER_LINE_2] = ModuleCommands::POWER_OFF;
//    mPowerSupplyRelayStates[ModuleCommands::DRIVE_CONTROL] = ModuleCommands::POWER_OFF;

//    mMKOPowerSupplyRelayStates[ModuleCommands::MKO_1] = ModuleCommands::POWER_OFF;
//    mMKOPowerSupplyRelayStates[ModuleCommands::MKO_2] = ModuleCommands::POWER_OFF;
//    mMKOPowerSupplyRelayStates[ModuleCommands::MKO_3] = ModuleCommands::POWER_OFF;
//    mMKOPowerSupplyRelayStates[ModuleCommands::MKO_4] = ModuleCommands::POWER_OFF;
}

ModuleSTM::~ModuleSTM()
{

}

void ModuleSTM::setPowerChannelState(ModuleCommands::PowerSupplyChannelID channel, ModuleCommands::PowerState state)
{
    addModuleCmd(ModuleCommands::SET_POWER_CHANNEL_STATE, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0);
}

void ModuleSTM::setMKOPowerChannelState(ModuleCommands::MKOPowerSupplyChannelID channel, ModuleCommands::PowerState state)
{
    addModuleCmd(ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, channel, (state == ModuleCommands::POWER_ON) ? 1 : 0);
}

//ModuleCommands::PowerState ModuleSTM::getPowerChannelState(ModuleCommands::PowerSupplyChannelID channel)
//{
//    return mPowerSupplyRelayStates.value(channel, ModuleCommands::POWER_OFF);
//}

//ModuleCommands::PowerState ModuleSTM::getMKOPowerChannelState(ModuleCommands::MKOPowerSupplyChannelID channel)
//{
//    return mMKOPowerSupplyRelayStates.value(channel, ModuleCommands::POWER_OFF);
//}

void ModuleSTM::getChannelTelemetry(int channel)
{
    if (channel < 0 || channel >= TELEMETRY_CHANNELS_COUNT)
    {
        LOG_ERROR(QString("Invalid channel id=%1 for getting telemetry").arg(channel));
        return;
    }

    mRequestedChannelTelemetry = channel;
    addModuleCmd(ModuleCommands::GET_CHANNEL_TELEMETRY, channel, 0);
}

ModuleSTM::FuseStates ModuleSTM::fuseState(int fuseIndex)
{
    // valid values 1 to 8 //TODO
    if (fuseIndex < 1 || fuseIndex > 8)
    {
        LOG_ERROR(QString("Invalid fuse index %1").arg(fuseIndex));
        return ModuleSTM::ERROR;
    }

    addModuleCmd(ModuleCommands::GET_FUSE_STATE, fuseIndex, 0);
//        LOG_ERROR(QString("Can not check fuse %1 state").arg(fuseIndex));
//        return ModuleSTM::ERROR;

    //TODO
    int state = 0; //response[3];

    QMetaEnum e = QMetaEnum::fromType<ModuleSTM::FuseStates>();
    LOG_INFO(QString("Fuse %1 state %2").arg(fuseIndex).arg(e.valueToKey(ModuleSTM::FuseStates(state))));

    return ModuleSTM::FuseStates(state); //TODO undefined values
}

void ModuleSTM::processCustomCommand()
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
        {
            int channel = mCurrentTransaction.inputParams.value(SystemState::CHANNEL_ID).toInt();
            int state = mCurrentTransaction.inputParams.value(SystemState::POWER_STATE).toInt();

            setPowerChannelState(ModuleCommands::PowerSupplyChannelID(channel), ModuleCommands::PowerState(state));
        }
        break;

    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
        {
            int channel = mCurrentTransaction.inputParams.value(SystemState::CHANNEL_ID).toInt();
            int state = mCurrentTransaction.inputParams.value(SystemState::POWER_STATE).toInt();

            setMKOPowerChannelState(ModuleCommands::MKOPowerSupplyChannelID(channel), ModuleCommands::PowerState(state));
        }
        break;

    default:
        LOG_ERROR(QString("Unexpected command %1 received by STM module").arg(command));
        break;
    }
}

bool ModuleSTM::processCustomResponse(uint32_t operationID, const QByteArray& request, const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(operationID);

    switch (command)
    {
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        {
            uint8_t uu1, uu2;
            uu1 = response[2];
            uu2 = response[3];
            qreal voltage = qreal((uu1 << 8) | uu2) / 10000;

            LOG_INFO(QString("Telemetry channel %1 voltage=%2").arg(mRequestedChannelTelemetry).arg(voltage));

            if (voltage >= CHANNEL_CONNECTED_BORDER)
            {

            }

            LOG_WARNING(QString("Get channel telemetry processing not implemented"));
            int TODO; //form response
        }
        break;

    case ModuleCommands::GET_POWER_CHANNEL_STATE:
        {
            QMetaEnum e = QMetaEnum::fromType<ModuleCommands::PowerSupplyChannelID>();
            uint8_t channel = response[2];
            uint8_t state = response[3];

            if (state == 0) // TODO move constants to encoder/decoder
            {
                LOG_INFO(QString("Power supply channel '%1' state is OFF").arg(e.valueToKey(channel)));
                //mPowerSupplyRelayStates[ModuleCommands::PowerSupplyChannelID(channel)] = ModuleCommands::POWER_OFF;
            }
            else if (state == 1)
            {
                LOG_INFO(QString("Power supply channel '%1' state is ON").arg(e.valueToKey(channel)));
                //mPowerSupplyRelayStates[ModuleCommands::PowerSupplyChannelID(channel)] = ModuleCommands::POWER_ON;
            }
            else if (state == 2) //TODO error is possibly be detected earlier
            {
                LOG_ERROR(QString("Couldn't get '%1' power supply channel state. Errror occured").arg(e.valueToKey(channel)));
            }

            int TODO; // form response
        }
        break;
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        {
            QMetaEnum e = QMetaEnum::fromType<ModuleCommands::MKOPowerSupplyChannelID>();
            uint8_t channel = response[2];
            uint8_t state = response[3];

            if (state == 0) // TODO move constants to encoder/decoder
            {
                LOG_INFO(QString("MKO power supply channel '%1' state is OFF").arg(e.valueToKey(channel)));
                //mMKOPowerSupplyRelayStates[ModuleCommands::MKOPowerSupplyChannelID(channel)] = ModuleCommands::POWER_OFF;
            }
            else if (state == 1)
            {
                LOG_INFO(QString("MKO power supply channel '%1' state is ON").arg(e.valueToKey(channel)));
                //mMKOPowerSupplyRelayStates[ModuleCommands::MKOPowerSupplyChannelID(channel)] = ModuleCommands::POWER_ON;
            }
            else if (state == 2) //TODO error is possibly be detected earlier
            {
                LOG_ERROR(QString("Couldn't get '%1' MKO power supply channel state. Errror occured").arg(e.valueToKey(channel)));
            }

            int TODO; // form response
        }
        break;

    case ModuleCommands::GET_FUSE_STATE:
        {
            int TODO;
        }
        break;

    case ModuleCommands::SET_POWER_CHANNEL_STATE:
        {
            int TODO;
        }
        break;
    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
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

void ModuleSTM::onModuleError()
{
    int TODO; //TODO here will be processing
}

void ModuleSTM::createResponse(Transaction& response)
{
    response = mCurrentTransaction;
}

void ModuleSTM::onApplicationFinish()
{
    int TODO; // check state and write errors/warnings

    closePort();
}
