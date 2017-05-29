#include "Headers/system/modules/module_mko.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QMetaEnum>

//#include <windows.h>
#include "Sources/system/WDMTMKv2.cpp"

/* Предполагаемая логика работы комплектов МКО:
 *
 * 1. По-нормальному питание на оба комплекта дается один раз на старте приложения и выключается при его завершении.
 * 2. В пользовательских циклограммах питание МКО дергаться не должно (если дергают - стопаем МКО с ошибкой).
 * 3. "Физически активным" МКО является в двух случаях:
 *    - если он не стартован (все команды, кроме startMKO, будут забриваться)
 *    - если запитаны и сконфигурированы оба комплекта (будут забриваться только если пытаться слать команды на незапитанные комплекты БУПа)
 * 4. Сообщения всегда шлются через один комплект. Если включено два, то шлется всегда в основной
 * 5. Можно слать и в оба, но хз как разрешать коллизии и откуда вычитывать ответ.
*/

ModuleMKO::KitState::KitState():
    isOnBUPKit(false),
    isOnMKOKit(false),
    isOnBUPDrives(false),
    isConfigured(false),
    isBCMode(false),
    isSelected(false),
    isReady(false),
    tmkID(0)
{
//    int tmkselected()

//    TMK_DATA tmkgetmode()

//    returns:
//    #define BC_MODE 0x00
//    #define RT_MODE 0x80
//    #define MT_MODE 0x100
//    #define MRT_MODE 0x280
//    #define UNDEFINED_MODE 0xFFFF
}

///////////////////////////////////

namespace
{
    static const uint16_t MAIN_KIT_ADDRESS = 0x1E;
    static const uint16_t RESERVE_KIT_ADDRESS = 0x1D;
    static const int RECEIVE_DELAY = 100; // msec
    static const int PROTECTION_DELAY = 100; // msec
    static const int LOCAL_MESSAGE_SEND_DELAY = 10; // (msec) delay for sending response to cyclogram in case of local soft/state errors
    static const int RECEIVE_BUFFER_SIZE = 100; // words
    static const int MAX_REPEAT_REQUESTS = 10; // Макс кол-во перезапросов при получении "Нет возможности обмена"

    static const QString ERR_WRONG_INFO = "Wrong info received"; // "Принята недостоверная информация"
    static const QString ERR_RESPONSE_NOT_READY = "Response not ready"; // "Нет возможности обмена"
    static const QString ERR_SUBSCRIBER_MALFUNC = "Subscriber malfunction"; // "Абонент неисправен"
    static const QString ERR_DEVICE_MALFUNCTION = "Terminal device works wrong"; // "ОУ функционирует неправильно"
    static const QString ERR_ADDR_MISMATCH = "Address mismatch"; //"Неверный адрес в ОС"
}

ModuleMKO::ModuleMKO(QObject* parent):
    AbstractModule(parent),
    mWordsToReceive(0),
    mWordsSent(0),
    mRepeatedRequests(0),
    mTMKOpened(false),
    mSwitchToMainKitAfterResponse(false)
{
    mMainKitState.address = MAIN_KIT_ADDRESS;
    mReserveKitState.address = RESERVE_KIT_ADDRESS;
}

ModuleMKO::~ModuleMKO()
{
    if (mTMKOpened)
    {
        LOG_ERROR(QString("MKO not stopped!"));
    }
}

void ModuleMKO::readResponse()
{
    // parsing response word for errors
    uint16_t buffer[RECEIVE_BUFFER_SIZE];
    bcgetblk(mWordsSent, &buffer, mWordsToReceive);

    QString dataStr;
    for (uint16_t i = 0; i < mWordsToReceive; ++i)
    {
        dataStr += QString::number(buffer[i], 16);
        dataStr += QString(" ");
    }

    LOG_INFO(QString("Receive data from MKO: %1").arg(dataStr));

    QStringList errors;
    processResponseWord(buffer[0], errors);

    // process response errors
    if (!errors.isEmpty())
    {
        for (auto it = errors.begin(); it != errors.end(); ++it)
        {
            LOG_ERROR(QString("MKO response word error: %1").arg(*it));
        }

        if (errors.size() == 1 && errors.at(0) == ERR_RESPONSE_NOT_READY)
        {
            if (mRepeatedRequests > MAX_REPEAT_REQUESTS)
            {
                mCurrentTransaction.error = QString("Max repeat count of %1 exceeded! Error: %2").arg(MAX_REPEAT_REQUESTS).arg(ERR_RESPONSE_NOT_READY);
            }
            else
            {
                ++mRepeatedRequests;
                LOG_WARNING(QString("Restarting receive timer..."));
                QTimer::singleShot(RECEIVE_DELAY, this, SLOT(resendRequest()));
                return;
            }
        }
        else
        {
            mCurrentTransaction.error = QString("Critial hardware errors received");
        }
    }

    // send response to cyclogram
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    mRepeatedRequests = 0;

    switch (command)
    {
    case ModuleCommands::SEND_TEST_ARRAY:
    case ModuleCommands::SEND_COMMAND_ARRAY:
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            // no special processing needed, just response word on OK/FAIL is enough
        }
        break;
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
        {
            if (mSwitchToMainKitAfterResponse)
            {
                mSwitchToMainKitAfterResponse = false;
                if (tmkselect(mMainKitState.tmkID) == 0)
                {
                    mMainKitState.isSelected = true;
                    mReserveKitState.isSelected = false;
                }
                else
                {
                    mCurrentTransaction.error = QString("Main MKO kit not selected after angle sensor power on via the reserve kit");
                }
            }
        }
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY:
    case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            for (uint16_t i = 1; i < mWordsToReceive; ++i) // 1 - skip response word
            {
                if (buffer[i] != 0)
                {
                    mCurrentTransaction.error = QString("Incorrect test array received");
                    break;
                }
            }
        }
        break;

    case ModuleCommands::RECEIVE_COMMAND_ARRAY:
        {
            int offset = 1;

            //QMetaEnum metaEnum = QMetaEnum::fromType<SystemState::ParamID>();

            QMap<uint32_t, QVariant> outputParams;
            for (auto it = mCurrentTransaction.outputParams.begin(); it != mCurrentTransaction.outputParams.end(); ++it)
            {
                uint32_t type = it.key();
                //QString variable = it.value().toString();
                qreal value = 0;

                switch (type)
                {
                case SystemState::MODE_PSY:
                    {
                        int16_t v = buffer[0 + offset];
                        value = v;
                    }
                    break;
                case SystemState::STEPS_PSY:
                    {
                        int16_t v1 = buffer[1 + offset];
                        int32_t v = buffer[2 + offset] + (v1 << 16);
                        value = v;
                    }
                    break;
                case SystemState::VELOCITY_PSY:
                    {
                        int16_t v = buffer[3 + offset];
                        value = v;
                    }
                    break;
                case SystemState::CURRENT_PSY:
                    {
                        int16_t v = buffer[4 + offset];
                        value = v;
                    }
                    break;
                case SystemState::ANGLE_PSY:
                    {
                        qreal rawData = buffer[10 + offset];
                        value = rawData * 180 / 65536;
                    }
                    break;
                case SystemState::MODE_NU:
                    {
                        int16_t v = buffer[5 + offset];
                        value = v;
                    }
                    break;
                case SystemState::STEPS_NU:
                    {
                        int16_t v1 = buffer[6 + offset];
                        int32_t v = buffer[7 + offset] + (v1 << 16);
                        value = v;
                    }
                    break;
                case SystemState::VELOCITY_NU:
                    {
                        int16_t v = buffer[8 + offset];
                        value = v;
                    }
                    break;
                case SystemState::CURRENT_NU:
                    {
                        int16_t v = buffer[9 + offset];
                        value = v;
                    }
                    break;
                case SystemState::ANGLE_NU:
                    {
                        qreal rawData = buffer[11 + offset];
                        value = rawData * 180 / 65536;
                    }
                    break;
                case SystemState::SENSOR_FLAG:
                    {
                        uint16_t tmp = buffer[19 + offset];
                        tmp = tmp << 3;
                        tmp = tmp >> 15;
                        value = tmp;
                    }
                    break;
                case SystemState::TEMPERATURE:
                    {
                        uint16_t tmp = buffer[19 + offset];
                        tmp = tmp << 3;
                        tmp = tmp >> 15;
                        if (tmp == 1) // has temperature sensor
                        {
                            int16_t temper = buffer[19 + offset] & 0x0fff; //TODO скорее всего какая-то хуйня, если это интерпретировать как int, а не float
                            value = temper;
                        }
                        else
                        {
                            value = 0xffff;
                        }
                    }
                    break;

                default:
                    LOG_ERROR(QString("Internal error occured in MKO response parsing"));
                    break;
                }

                //LOG_DEBUG(QString("%1: %2 = %3").arg(metaEnum.valueToKey(type)).arg(variable).arg(value));

                //TODO replace by addResponseParam();
                QList<QVariant> list;
                list.append(it.value());
                list.append(value);
                outputParams[it.key()] = list;
            }

            mCurrentTransaction.outputParams = outputParams;
        }
        break;

    case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int offset = 1;
            QMap<uint32_t, QVariant> outputParams;
            for (auto it = mCurrentTransaction.outputParams.begin(); it != mCurrentTransaction.outputParams.end(); ++it)
            {
                uint32_t type = it.key();
                QString variable = it.value().toString();
                qreal value = 0;

                LOG_DEBUG(QString("Param type %1 variable name is %2").arg(type).arg(variable));

                switch (type)
                {
                case SystemState::DRIVE_MODE:
                    {
                        int16_t v = buffer[0 + offset];
                        value = v;
                    }
                    break;
                case SystemState::STEPS:
                    {
                        int16_t v1 = buffer[1 + offset];
                        int32_t v = buffer[2 + offset] + (v1 << 16);
                        value = v;
                    }
                    break;
                case SystemState::VELOCITY:
                    {
                        int16_t v = buffer[3 + offset];
                        value = v;
                    }
                    break;
                case SystemState::CURRENT:
                    {
                        int16_t v = buffer[4 + offset];
                        value = v;
                    }
                    break;

                default:
                    LOG_ERROR(QString("Internal error occured in MKO response parsing"));
                    break;
                }

                //TODO replace by addResponseParam();
                QList<QVariant> list;
                list.append(it.value());
                list.append(value);
                outputParams[it.key()] = list;
            }

            mCurrentTransaction.outputParams = outputParams;
        }
        break;

    default:
        LOG_ERROR(QString("MKO unknown command error"));
        break;
    }

    QTimer::singleShot(PROTECTION_DELAY, this, SLOT(sendResponse()));
}

void ModuleMKO::sendResponse()
{
    emit commandResult(mCurrentTransaction);
}

void ModuleMKO::sendLocalMessage(const QString& error)
{
    mCurrentTransaction.error = error;
    QTimer::singleShot(LOCAL_MESSAGE_SEND_DELAY, this, SLOT(sendResponse()));
}

void ModuleMKO::resendRequest()
{
    LOG_WARNING(QString("Resending request..."));
    Transaction tmp = mCurrentTransaction;
    processCommand(tmp);
}

void ModuleMKO::startMKO()
{
    LOG_INFO(QString("Starting MKO..."));

    // 1. Check MKO kits power supply is on. The must be both enbled
    if (!mMainKitState.isOnMKOKit)
    {
        sendLocalMessage(QString("MKO main kit power supply is off"));
        return;
    }

    if (!mReserveKitState.isOnMKOKit)
    {
        sendLocalMessage(QString("MKO reserve kit power supply is off"));
        return;
    }

    // 2.Dynamically load device driver library
    if (!mTMKOpened && TmkOpen() != 0)
    {
        sendLocalMessage(QString("TMK library not loaded"));
        return;
    }

    mTMKOpened = true;

    // 3.Configure MKO main kit (it also selects this kit as "current")
    if (!mMainKitState.isConfigured && tmkconfig(mMainKitState.tmkID) != 0)
    {
        sendLocalMessage(QString("TMK main kit device (%1) configuration failed").arg(mMainKitState.tmkID));
        return;
    }

    mMainKitState.isConfigured = true;

    // 4.Reset MKO main kit to "channel controller" mode
    if (!mMainKitState.isBCMode && bcreset() != 0)
    {
        sendLocalMessage(QString("TMK main kit device (%1) set to 'channel controller' mode failed").arg(mMainKitState.tmkID));
        return;
    }

    mMainKitState.isBCMode = true;
    mMainKitState.isReady = true; // Main kit is ready to send and receive messages

    // 5.Configure MKO reserve kit (it also selects this kit as "current")
    if (!mReserveKitState.isConfigured && tmkconfig(mReserveKitState.tmkID) != 0)
    {
        sendLocalMessage(QString("TMK reserve kit device (%1) configuration failed").arg(mReserveKitState.tmkID));
        return;
    }

    mReserveKitState.isConfigured = true;

    // 6.Reset MKO reserve kit to "channel controller" mode
    if (!mReserveKitState.isBCMode && bcreset() != 0)
    {
        sendLocalMessage(QString("TMK reserve kit device (%1) set to 'channel controller' mode failed").arg(mReserveKitState.tmkID));
        return;
    }

    mReserveKitState.isBCMode = true;
    mReserveKitState.isReady = true; // Reserve kit is ready to send and receive messages

    // 7. Select main kit as current device
    if (!mMainKitState.isSelected && tmkselect(mMainKitState.tmkID) != 0)
    {
        sendLocalMessage(QString("TMK main kit device (%1) not selected").arg(mMainKitState.tmkID));
        return;
    }

    mMainKitState.isSelected = true;
    mReserveKitState.isSelected = false;

    LOG_INFO(QString("MKO started"));

    sendLocalMessage();
}

void ModuleMKO::stopMKO(bool onAppFinish)
{
    if (!mTMKOpened)
    {
        if (!onAppFinish)
        {
            LOG_WARNING(QString("Trying to stop MKO, when it is already stopped"));
            sendLocalMessage();
        }

        return;
    }
    else if (onAppFinish)
    {
        LOG_WARNING(QString("MKO started, but was not stopped with application finish cyclogram!"));
    }

    LOG_INFO(QString("Stopping MKO..."));

    if (mMainKitState.isSelected || mReserveKitState.isSelected)
    {
        bcreset(); //TODO it was in example, I dont'k know for, maybe for some implicit USB cleanup (in addition to reset to BC mode)
    }

    // 1. Release all TMK devices
    tmkdone(ALL_TMKS);

    // 2. Clear devices states
    mMainKitState.isBCMode = false;
    mMainKitState.isConfigured = false;
    mMainKitState.isReady = false;
    mMainKitState.isSelected = false;

    mReserveKitState.isBCMode = false;
    mReserveKitState.isConfigured = false;
    mReserveKitState.isReady = false;
    mReserveKitState.isSelected = false;

    // 3. Unload device driver library
    TmkClose();

    mTMKOpened = false;

    LOG_INFO(QString("MKO stopped"));

    sendLocalMessage();
}

void ModuleMKO::processResponseWord(uint16_t responseWord, QStringList& errors)
{
    LOG_INFO(QString("Response word is 0x%1").arg(QString::number(responseWord, 16)));

    uint16_t address;
    if (mMainKitState.isSelected)
    {
        address = mMainKitState.address;
    }
    else if (mReserveKitState.isSelected)
    {
        address = mReserveKitState.address;
    }
    else
    {
        LOG_ERROR(QString("Response received but no enabled BUP kit found"));
    }

    int TODO; // what in case of two kits are enabled simultaneously?

    if (responseWord >> 11 != address)
    {
        LOG_ERROR(QString("Addr from OS: %1, Addr cur: %2").arg(responseWord >> 11).arg(address));
        errors.append(ERR_ADDR_MISMATCH);
    }

    //TODO use ADDRESS_MASK etc instead of bit shifts
    uint16_t x;
    x = responseWord << 5;
    x = x >> 15;
    if (x == 1)
    {
        errors.append(ERR_WRONG_INFO);
    }

    x = responseWord << 12;
    x = x >> 15;

    if (x == 1)
    {
        errors.append(ERR_RESPONSE_NOT_READY);
    }

    x = responseWord << 13;
    x = x >> 15;

    if (x == 1)
    {
        errors.append(ERR_SUBSCRIBER_MALFUNC);
    }

    x = responseWord << 15;
    x = x >> 15;

    if (x == 1)
    {
        errors.append(ERR_DEVICE_MALFUNCTION);
    }
}

void ModuleMKO::sendDataToBUP(uint16_t address, uint16_t subaddress, uint16_t* data, uint16_t wordsCount)
{
    LOG_INFO(QString("MKO: Send data to BUP: Address: %1, Subaddress: %2").arg(address).arg(subaddress));

    QString dataStr;

    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        dataStr += QString::number(data[i], 16);
        dataStr += QString(" ");
    }

    if (!dataStr.isEmpty())
    {
        LOG_INFO(QString("Data: %1").arg(dataStr));
    }

    //Why not use #define CW(ADDR,DIR,SUBADDR,NWORDS) ((TMK_DATA)(((ADDR)<<11)|(DIR)|((SUBADDR)<<5)|((NWORDS)&0x1F))) ?

    uint16_t commandWord = (address << 11) + RT_RECEIVE + (subaddress << 5) + (wordsCount & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, commandWord);
    bcputblk(1, data, wordsCount);
    bcstartx(0, DATA_BC_RT | CX_BUS_0 | CX_STOP | CX_NOSIG);

    QTimer::singleShot(RECEIVE_DELAY, this, SLOT(readResponse()));
}

void ModuleMKO::requestDataFromBUP(uint16_t address, uint16_t subaddress, uint16_t expectedWordsInResponse)
{
    LOG_INFO(QString("MKO: Request data from BUP: Address: %1, Subaddress: %2").arg(address).arg(subaddress));

    uint16_t commandWord = (address << 11) + RT_TRANSMIT + (subaddress << 5) + ((expectedWordsInResponse - 1) & NWORDS_MASK); //TODO '-1' - "response word" or "checksum"
    bcdefbase(0);
    bcputw(0, commandWord);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);

    QTimer::singleShot(RECEIVE_DELAY, this, SLOT(readResponse()));
}

void ModuleMKO::sendTestArray(uint16_t address)
{
    uint16_t data[11];
    uint16_t wordsCount = 10;
    uint16_t checkSum = 0;

    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        data[i] = 0;
        checkSum += data[i];
    }

    data[wordsCount] = checkSum;
    ++wordsCount;

    mWordsToReceive = 1;
    mWordsSent = 1 + wordsCount; // control message + data
    sendDataToBUP(address, SEND_SUBADDRESS, data, wordsCount);
}

void ModuleMKO::receiveTestArray(uint16_t address)
{
    mWordsToReceive = 12;
    mWordsSent = 1; // control message
    requestDataFromBUP(address, SEND_SUBADDRESS, mWordsToReceive);
}

void ModuleMKO::sendCommandArray(uint16_t address, const AxisData& psy, const AxisData& nu)
{
    uint16_t data[11];
    uint16_t wordsCount = 10;
    uint16_t checkSum = 0;

    data[0] = psy.mode;
    data[1] = psy.steps >> 16;
    data[2] = psy.steps;
    data[3] = psy.velocity;
    data[4] = psy.current;
    data[5] = nu.mode;
    data[6] = nu.steps >> 16;
    data[7] = nu.steps;
    data[8] = nu.velocity;
    data[9] = nu.current;

    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        checkSum += data[i];
    }

    data[wordsCount] = checkSum;
    ++wordsCount;

    mWordsToReceive = 1;
    mWordsSent = 1 + wordsCount; // control message + data
    sendDataToBUP(address, SEND_SUBADDRESS, data, wordsCount);
}

void ModuleMKO::receiveCommandArray(uint16_t address)
{
    mWordsToReceive = 22;
    mWordsSent = 1; // control message
    requestDataFromBUP(address, RECEIVE_SUBADDRESS, mWordsToReceive);
}

void ModuleMKO::sendTestArrayForChannel(uint16_t address, Subaddress channel)
{
    uint16_t data[6];
    uint16_t wordsCount = 5;
    uint16_t checkSum = 0;

    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        data[i] = 0;
        checkSum += data[i];
    }

    data[wordsCount] = checkSum;
    ++wordsCount;

    mWordsToReceive = 1;
    mWordsSent = 1 + wordsCount; // control message + data
    sendDataToBUP(address, channel, data, wordsCount);
}

void ModuleMKO::receiveTestArrayForChannel(uint16_t address, Subaddress channel)
{
    mWordsToReceive = 6;
    mWordsSent = 1; // control message
    requestDataFromBUP(address, channel, mWordsToReceive);
}

void ModuleMKO::sendCommandArrayForChannel(uint16_t address, Subaddress channel, const AxisData& axisData)
{
    uint16_t data[6];
    uint16_t wordsCount = 5;
    uint16_t checkSum = 0;

    data[0] = axisData.mode;
    data[1] = axisData.steps >> 16;
    data[2] = axisData.steps;
    data[3] = axisData.velocity;
    data[4] = axisData.current;

    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        checkSum += data[i];
    }

    data[wordsCount] = checkSum;
    ++wordsCount;

    mWordsToReceive = 1;
    mWordsSent = 1 + wordsCount; // control message + data
    sendDataToBUP(address, channel, data, wordsCount);
}

void ModuleMKO::receiveCommandArrayForChannel(uint16_t address, Subaddress channel)
{
    //TODO channel param not used
    mWordsToReceive = 6;
    mWordsSent = 1; // control message
    requestDataFromBUP(address, RECEIVE_SUBADDRESS, mWordsToReceive);
}

QString ModuleMKO::prepareSendToAngleSensor(uint16_t& address, AngleSensorPowerSupplySource source)
{
    if (source == PS_FROM_MAIN_KIT)
    {
        address = mMainKitState.address;

        if (mMainKitState.isSelected) // main kit selected, no special preparation needed
        {
            return "";
        }

        if (!mReserveKitState.isSelected)
        {
            return QString("No MKO kit selected. Can not send angle sensor power supply command via main MKO kit");
        }

        if (!mMainKitState.isOnBUPKit)
        {
            return QString("Trying to send angle sensor power supply command to deenergized main kit");
        }

        if (tmkselect(mMainKitState.tmkID) != 0)
        {
            return QString("Could not select main MKO kit");
        }

        // main MKO kit selected, now we can send command
        mMainKitState.isSelected = true;
        mReserveKitState.isSelected = false;
    }
    else if (source == PS_FROM_RESERVE_KIT)
    {
        address = mReserveKitState.address;

        if (mReserveKitState.isSelected)
        {
            return ""; // reserve kit selected, no special preparation needed
        }

        if (!mMainKitState.isSelected)
        {
            return QString("No MKO kit selected. Can not send angle sensor power supply command via reserve MKO kit");
        }

        if (!mReserveKitState.isOnBUPKit)
        {
            return QString("Trying to send angle sensor power supply command to deenergized reserve kit");
        }

        if (tmkselect(mReserveKitState.tmkID) != 0)
        {
            return QString("Could not select reserve MKO kit");
        }

        mMainKitState.isSelected = false;
        mReserveKitState.isSelected = true;
        mSwitchToMainKitAfterResponse = true; // set flag that we need to switch back to main MKO kit after response receive
    }
    else
    {
        return QString("Unknow power supply source for angle sensor");
    }

    return "";
}

void ModuleMKO::sendAngleSensorData(uint16_t address, AngleSensorPowerSupplySource source)
{
    uint16_t wordsCount = 2;

    uint16_t data[3];
    data[0] = 0;
    data[1] = source;

    // add checksum
    uint16_t checkSum = 0;
    for (uint16_t i = 0; i < wordsCount; ++i)
    {
        checkSum += data[i];
    }

    data[wordsCount] = checkSum;
    ++wordsCount;

    mWordsToReceive = 1;
    mWordsSent = 1 + wordsCount; // control message + data
    sendDataToBUP(address, ANGLE_SENSOR_SUBADDRESS, data, wordsCount);
}

QString ModuleMKO::canSendRequest(const Transaction& request) const
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(request.commandID);

    if (command == ModuleCommands::START_MKO || command == ModuleCommands::STOP_MKO)
    {
        return "";
    }

    // all commands excluding start/stop MKO must pass this check
    if (!mTMKOpened)
    {
        return QString("MKO not started");
    }

    if (!mMainKitState.isSelected && !mReserveKitState.isSelected)
    {
        return QString("No MKO Kit enabled");
    }

    if (!mMainKitState.isOnBUPDrives || !mReserveKitState.isOnBUPDrives)
    {
        return QString("BUP drives power supply is off");
    }

    if (!mMainKitState.isOnBUPKit && !mReserveKitState.isOnBUPKit)
    {
        return QString("No BUP kit enabled");
    }

    if (mMainKitState.isSelected && !mMainKitState.isOnBUPKit)
    {
        return QString("MKO internal main kit logic error 1");
    }

    if (!mMainKitState.isSelected && mMainKitState.isOnBUPKit)
    {
        return QString("MKO internal main kit logic error 2");
    }

    if (mReserveKitState.isSelected && !mReserveKitState.isOnBUPKit)
    {
        return QString("MKO internal reserve kit logic error 1");
    }

    if (!mReserveKitState.isSelected && mReserveKitState.isOnBUPKit)
    {
        return QString("MKO internal reserve kit logic error 2");
    }

    return "";
}

void ModuleMKO::processCommand(const Transaction& request)
{
    mCurrentTransaction.clear();
    mCurrentTransaction = request;

    QString error = canSendRequest(request);
    if (!error.isEmpty())
    {
        sendLocalMessage(error);
        return;
    }

    uint16_t address = 0;
    if (mMainKitState.isSelected)
    {
        address = mMainKitState.address;
    }
    else if (mReserveKitState.isSelected)
    {
        address = mReserveKitState.address;
    }

    ModuleCommands::CommandID command = ModuleCommands::CommandID(mCurrentTransaction.commandID);

    switch (command)
    {
    case ModuleCommands::SEND_TEST_ARRAY:
        {
            sendTestArray(address);
        }
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY:
        {
            receiveTestArray(address);
        }
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY:
        {
            AxisData psy;
            AxisData nu;

            for (auto it = mCurrentTransaction.inputParams.begin(); it != mCurrentTransaction.inputParams.end(); ++it)
            {
                uint32_t type = it.key();
                int32_t value = int32_t(it.value().toDouble());

                switch (type)
                {
                case SystemState::MODE_PSY:
                    psy.mode = value;
                    break;
                case SystemState::STEPS_PSY:
                    psy.steps = value;
                    break;
                case SystemState::VELOCITY_PSY:
                    psy.velocity = value;
                    break;
                case SystemState::CURRENT_PSY:
                    psy.current = value;
                    break;
                case SystemState::MODE_NU:
                    nu.mode = value;
                    break;
                case SystemState::STEPS_NU:
                    nu.steps = value;
                    break;
                case SystemState::VELOCITY_NU:
                    nu.velocity = value;
                    break;
                case SystemState::CURRENT_NU:
                    nu.current = value;
                    break;
                default:
                    break;
                }
            }

            sendCommandArray(address, psy, nu);
        }
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY:
        {
            receiveCommandArray(address);
        }
        break;
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
        {
            Subaddress subaddress = Subaddress(mCurrentTransaction.inputParams.value(SystemState::SUBADDRESS).toInt());
            sendTestArrayForChannel(address, subaddress);
        }
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            Subaddress subaddress = Subaddress(mCurrentTransaction.inputParams.value(SystemState::SUBADDRESS).toInt());
            receiveTestArrayForChannel(address, subaddress);
        }
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            AxisData data;
            for (auto it = mCurrentTransaction.inputParams.begin(); it != mCurrentTransaction.inputParams.end(); ++it)
            {
                uint32_t type = it.key();
                int32_t value = int32_t(it.value().toDouble());

                switch (type)
                {
                case SystemState::DRIVE_MODE:
                    data.mode = value;
                    break;
                case SystemState::STEPS:
                    data.steps = value;
                    break;
                case SystemState::VELOCITY:
                    data.velocity = value;
                    break;
                case SystemState::CURRENT:
                    data.current = value;
                    break;
                default:
                    break;
                }
            }

            Subaddress subaddress = Subaddress(mCurrentTransaction.inputParams.value(SystemState::SUBADDRESS).toInt());
            sendCommandArrayForChannel(address, subaddress, data);
        }
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            Subaddress subaddress = Subaddress(mCurrentTransaction.inputParams.value(SystemState::SUBADDRESS).toInt());
            receiveCommandArrayForChannel(address, subaddress);
        }
        break;
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
        {
            AngleSensorPowerSupplySource source = AngleSensorPowerSupplySource(mCurrentTransaction.inputParams.value(SystemState::SUBADDRESS).toInt());

            QString error = prepareSendToAngleSensor(address, source);
            if (error.isEmpty())
            {
                sendAngleSensorData(address, source);
            }
            else
            {
                sendLocalMessage(error);
                return;
            }
        }
        break;

    case ModuleCommands::START_MKO:
        {
            startMKO();
        }
        break;

    case ModuleCommands::STOP_MKO:
        {
            stopMKO(false);
        }
        break;

    default:
        {
            sendLocalMessage(QString("MKO: Unknown command id=%1").arg(int(command)));
            return;
        }
        break;
    }

    if (mWordsToReceive > RECEIVE_BUFFER_SIZE)
    {
        sendLocalMessage(QString("MKO: Receive buffer overflow: Requred size=%1, Available size=%2").arg(mWordsToReceive).arg(RECEIVE_BUFFER_SIZE));
        return;
    }
}

void ModuleMKO::onApplicationFinish()
{
    stopMKO(true);
}

bool ModuleMKO::isPhysicallyActive() const
{
    if (mTMKOpened)
    {
        return (mMainKitState.isReady && mReserveKitState.isReady);
    }

    return true; // if startMKO() not called, module is physically active to receive startMKO() calls
}

void ModuleMKO::onPowerRelayStateChanged(ModuleCommands::PowerSupplyChannelID channel, ModuleCommands::PowerState state)
{
    switch (channel)
    {
    case ModuleCommands::BUP_MAIN:
        {
            mMainKitState.isOnBUPKit = (state == ModuleCommands::POWER_ON);

            if (mTMKOpened && isPhysicallyActive())
            {
                if (mMainKitState.isOnBUPKit) // if main BUP kit become enabled, switch MKO to it
                {
                    if (tmkselect(mMainKitState.tmkID) == 0)
                    {
                        mMainKitState.isSelected = true;
                    }
                    else
                    {
                        LOG_ERROR(QString("MKO main kit not selected"));
                        mMainKitState.isSelected = false;
                    }

                    mReserveKitState.isSelected = false;
                }
                else if (mReserveKitState.isOnBUPKit)  // if main BUP kit become disabled and BUP reserve kit is still enable
                {
                    if (tmkselect(mReserveKitState.tmkID) == 0)
                    {
                        mReserveKitState.isSelected = true;
                    }
                    else
                    {
                        LOG_ERROR(QString("MKO reserve kit not selected 1"));
                        mReserveKitState.isSelected = false;
                    }

                    mMainKitState.isSelected = false;
                }
            }
        }
        break;
    case ModuleCommands::BUP_RESERVE:
        {
            mReserveKitState.isOnBUPKit = (state == ModuleCommands::POWER_ON);

            if (mTMKOpened && isPhysicallyActive())
            {
                if (mReserveKitState.isOnBUPKit) // if reserve BUP kit become enabled
                {
                    if (!mMainKitState.isOnBUPKit) // if main BUP kit not enabled, switch MKO to reserve kit
                    {
                        if (tmkselect(mReserveKitState.tmkID) == 0)
                        {
                            mReserveKitState.isSelected = true;
                        }
                        else
                        {
                            LOG_ERROR(QString("MKO reserve kit not selected 2"));
                            mReserveKitState.isSelected = false;
                        }
                    }
                    else
                    {
                        mReserveKitState.isSelected = false;
                    }
                }
                else
                {
                    mReserveKitState.isSelected = false;
                }
            }
        }
        break;
    case ModuleCommands::DRIVE_CONTROL:
        {
            mMainKitState.isOnBUPDrives = (state == ModuleCommands::POWER_ON);
            mReserveKitState.isOnBUPDrives = (state == ModuleCommands::POWER_ON);
            // selecting MKO kits not performs, because these flags affect only to ability to send commands to BUP
        }
        break;

    // these relay states changing is not interesting to MKO
    case ModuleCommands::RESERVED_RELAY_3:
    case ModuleCommands::HEATER_LINE_1:
    case ModuleCommands::HEATER_LINE_2:
        break;
    default:
        break;
    }
}

void ModuleMKO::updateMKO(KitState& changedKit, bool isOn, const KitState& otherKit)
{
    bool wasOn = changedKit.isOnMKOKit;
    changedKit.isOnMKOKit = isOn;

    QString kitName;
    if (changedKit.address == MAIN_KIT_ADDRESS)
    {
        kitName = "main";
    }
    else
    {
        kitName = "reserve";
    }

    if (wasOn && !isOn) // Active MKO kit power supply was switched off
    {
        if (mTMKOpened)
        {
            LOG_ERROR(QString("Stopping MKO because MKO %1 kit power supply was off. Both main and reserve MKO kits must be enabled!").arg(kitName));
            stopMKO(false); // disable entire MKO kits if one of them was disabled
            return;
        }
        else // MKO is stopped before power supply is off, normal situation
        {

        }
    }

    if (!wasOn && isOn) // Inactive MKO kit power supply was switched on
    {
        if (!mTMKOpened) // MKO kit power supply was on before startMKO call
        {
            changedKit.tmkID = (otherKit.isOnMKOKit ? 1 : 0);
        }
        else // MKO kit was previously power off, try call startMKO one more time
        {
            LOG_ERROR(QString("Unexpected MKO state %1 kit was switched off!").arg(kitName));
        }

        return;
    }
}

void ModuleMKO::onPowerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID channel, ModuleCommands::PowerState state)
{
    switch (channel)
    {
    case ModuleCommands::MKO_1:
        {
            updateMKO(mMainKitState, (state == ModuleCommands::POWER_ON), mReserveKitState);
        }
        break;

    case ModuleCommands::MKO_2:
        {
            updateMKO(mReserveKitState, (state == ModuleCommands::POWER_ON), mMainKitState);
        }
        break;

    // these relay states changing is not interesting to MKO
    case ModuleCommands::MKO_3:
    case ModuleCommands::MKO_4:
        break;
    default:
        break;
    }
}
