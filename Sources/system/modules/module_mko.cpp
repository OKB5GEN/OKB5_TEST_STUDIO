#include "Headers/system/modules/module_mko.h"
#include "Headers/system/system_state.h"
#include "Headers/logger/Logger.h"

#include <QTimer>

#include <windows.h>
#include "Sources/system/WDMTMKv2.cpp"


HANDLE hEvent, hEvent1; //TODO some internal MKO shit here

namespace
{
    static const uint16_t MAIN_KIT_ADDRESS = 0x1E;
    static const uint16_t RESERVE_KIT_ADDRESS = 0x1D;
    static const int RECEIVE_DELAY = 100; // msec
    static const int PROTECTION_DELAY = 100; // msec
    static const int RECEIVE_BUFFER_SIZE = 100; // words
    static const int MAX_REPEAT_REQUESTS = 5; // Макс кол-во перезапросов при получении "Нет возможности обмена"

    static const QString ERR_WRONG_INFO = "Wrong info received"; // "Принята недостоверная информация"
    static const QString ERR_RESPONSE_NOT_READY = "Response not ready"; // "Нет возможности обмена"
    static const QString ERR_SUBSCRIBER_MALFUNC = "Subscriber malfunction"; // "Абонент неисправен"
    static const QString ERR_DEVICE_MALFUNCTION = "Terminal device works wrong"; // "ОУ функционирует неправильно"
    static const QString ERR_ADDR_MISMATCH = "Address mismatch"; //"Неверный адрес в ОС"
}

ModuleMKO::ModuleMKO(QObject* parent):
    AbstractModule(parent),
    mMainKitEnabled(false),
    mReserveKitEnabled(false),
    mWordsToReceive(0),
    mWordsSent(0),
    mActiveKits(NO_KIT),
    mRepeatedRequests(0)
{
}

ModuleMKO::~ModuleMKO()
{
    //stopMKO(); // TODO remove
}

void ModuleMKO::readResponse()
{
    // хитрожопая логика вычитывания: надо вычиывать столько, сколько послал и после этого будут лежать ответное слово + данные (если есть)
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
            LOG_ERROR(QString("MKO response error: %1").arg(*it));
        }

        if (errors.size() == 1 && errors.at(0) == ERR_RESPONSE_NOT_READY)
        {
            if (mRepeatedRequests > MAX_REPEAT_REQUESTS)
            {
                LOG_ERROR(QString("Max repeat count of %1 exceeded! Error: %2").arg(MAX_REPEAT_REQUESTS).arg(ERR_RESPONSE_NOT_READY));
                mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(305)); //TODO define error codes internal or hardware
            }
            else
            {
                ++mRepeatedRequests;
                QTimer::singleShot(RECEIVE_DELAY, this, SLOT(readResponse()));
                return;
            }
        }
        else
        {
            mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(305)); //TODO define error codes internal or hardware
        }
    }

    // send response to cyclogram
    ModuleMKO::CommandID command = ModuleMKO::CommandID(mCurrentResponse.value(SystemState::COMMAND_ID).toUInt());

    mRepeatedRequests = 0;

    switch (command)
    {
    case SEND_TEST_ARRAY:
    case SEND_COMMAND_ARRAY:
    case SEND_TEST_ARRAY_FOR_CHANNEL:
    case SEND_COMMAND_ARRAY_FOR_CHANNEL:
    case SEND_TO_ANGLE_SENSOR:
        {
            // no special processing needed, just response word on OK/FAIL is enough
        }
        break;
    case RECEIVE_TEST_ARRAY:
    case RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            for (uint16_t i = 1; i < mWordsToReceive; ++i) // 1 - skip response word
            {
                if (buffer[i] != 0)
                {
                    LOG_ERROR(QString("Incorrect test array received"));
                    mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(405)); //TODO define error codes internal or hardware
                    break;
                }
            }
        }
        break;

    case RECEIVE_COMMAND_ARRAY:
        {
            LOG_DEBUG("Receive command addray");
            int offset = 1;

            int paramsCount = mCurrentResponse.value(SystemState::OUTPUT_PARAMS_COUNT).toInt() * 2;
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i).toInt();
                    QString variable = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i + 1).toString();
                    qreal value = 0;

                    LOG_DEBUG(QString("Param type %1 variable name is %2").arg(type).arg(variable));

                    switch (type)
                    {
                    case SystemState::MODE_PSY:
                        {
                            int16_t v = buffer[0 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("MODE_PSY").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[0]=%2").arg(v).arg(buffer[0 + offset]));
                        }
                        break;
                    case SystemState::STEPS_PSY:
                        {
                            int16_t v1 = buffer[1 + offset];
                            int32_t v = buffer[2 + offset] + (v1 << 16);
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("STEPS_PSY").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[1]=%2 buffer[2]=%3").arg(v).arg(buffer[1]).arg(buffer[2 + offset]));
                        }
                        break;
                    case SystemState::VELOCITY_PSY:
                        {
                            int16_t v = buffer[3 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("VELOCITY_PSY").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[3]=%2").arg(v).arg(buffer[3 + offset]));
                        }
                        break;
                    case SystemState::CURRENT_PSY:
                        {
                            int16_t v = buffer[4 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("CURRENT_PSY").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[4]=%2").arg(v).arg(buffer[4 + offset]));
                        }
                        break;
                    case SystemState::ANGLE_PSY:
                        {
                            qreal rawData = buffer[10 + offset];
                            value = rawData * 180 / 65536;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("ANGLE_PSY").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[10]=%2").arg(rawData).arg(buffer[10 + offset]));
                        }
                        break;
                    case SystemState::MODE_NU:
                        {
                            int16_t v = buffer[5 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("MODE_NU").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[5]=%2").arg(v).arg(buffer[5 + offset]));
                        }
                        break;
                    case SystemState::STEPS_NU:
                        {
                            int16_t v1 = buffer[6 + offset];
                            int32_t v = buffer[7 + offset] + (v1 << 16);
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("STEPS_NU").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[6]=%2 buffer[7]=%3").arg(v).arg(buffer[6 + offset]).arg(buffer[7 + offset]));
                        }
                        break;
                    case SystemState::VELOCITY_NU:
                        {
                            int16_t v = buffer[8 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("VELOCITY_NU").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[8]=%2").arg(v).arg(buffer[8 + offset]));
                        }
                        break;
                    case SystemState::CURRENT_NU:
                        {
                            int16_t v = buffer[9 + offset];
                            value = v;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("CURRENT_NU").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[9]=%2").arg(v).arg(buffer[9 + offset]));
                        }
                        break;
                    case SystemState::ANGLE_NU:
                        {
                            qreal rawData = buffer[11 + offset];
                            value = rawData * 180 / 65536;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("ANGLE_NU").arg(value));
                            LOG_DEBUG(QString("v=%1 buffer[11]=%2").arg(rawData).arg(buffer[11 + offset]));
                        }
                        break;
                    case SystemState::SENSOR_FLAG:
                        {
                            uint16_t tmp = buffer[19 + offset];
                            tmp = tmp << 3;
                            tmp = tmp >> 15;
                            value = tmp;
                            LOG_DEBUG(QString("Param %1 value is %2").arg("SENSOR_FLAG").arg(value));
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

                            LOG_DEBUG(QString("Param %1 value is %2").arg("TEMPERATURE").arg(value));
                            LOG_DEBUG(QString("temper=%1 buffer[19]=%2").arg(buffer[19] & 0x0fff).arg(buffer[19 + offset]));
                        }
                        break;

                    default:
                        LOG_ERROR(QString("Internal error occured in MKO response parsing"));
                        break;
                    }

                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = QVariant(variable);
                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i + 1] = QVariant(value);
                }

                mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);
            }
        }
        break;

    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int offset = 1;
            int paramsCount = mCurrentResponse.value(SystemState::OUTPUT_PARAMS_COUNT).toInt() * 2;
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i).toInt();
                    QString variable = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i + 1).toString();
                    qreal value = 0;

                    switch (type)
                    {
                    case SystemState::MODE:
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

                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = QVariant(variable);
                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i + 1] = QVariant(value);
                }
            }

            mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);
        }
        break;

    default:
        LOG_ERROR(QString("MKO messages internal error"));
        break;
    }

    QTimer::singleShot(PROTECTION_DELAY, this, SLOT(sendResponse()));
}

void ModuleMKO::sendResponse()
{
    emit commandResult(mCurrentResponse);
}

void ModuleMKO::startMKO1()
{
    QString data;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hEvent1)
    {
        data += "МКО: CreateEvent1() не запустился!\n";
        LOG_ERROR("CreateEvent() failed!");
    }

    if (!hEvent)
    {
        data += "МКО: CreateEvent() не запустился!\n";
        LOG_ERROR("CreateEvent() failed!");
    }

    if (TmkOpen())
    {
        data += "МКО: Библиотека не запустилась!\n";
        LOG_ERROR("TmkOpen() failed!");
    }
    else
    {
        LOG_INFO("TmkOpen() successful!");
    }

    if (tmkconfig(1) != 0)
    {
        data += "МКО: Конфигурация МКО 1 не произошла!\n";
        LOG_ERROR("MKO 1 config failed");
    }
    else
    {
        LOG_INFO("tmkconfig1() successful!");
    }

    if (tmkconfig(0) != 0)
    {
        data += "МКО: Конфигурация МКО 0 не произошла!\n";
        LOG_ERROR("MKO 0 config failed");
    }
    else
    {
        LOG_INFO("tmkconfig0() successful!\n");
    }

    if (bcreset() != 0)
    {
        data += "МКО: Перезагрузка МКО не произошла!\n";
        LOG_ERROR("MKO reset failed");
    }
}

void ModuleMKO::startMKO()
{
    QString data;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hEvent)
    {
        data += "МКО: CreateEvent() не запустился!\n";
        LOG_ERROR("CreateEvent() failed!");
    }

    if (TmkOpen())
    {
        data += "МКО: Библиотека МКО не запустилась!\n";
        LOG_ERROR("TmkOpen() failed!");
    }
    else
    {
        LOG_INFO("TmkOpen() successful!");
    }

    if (tmkconfig(0) != 0)
    {
        data += "МКО: Конфигурация МКО не произошла!\n";
        LOG_ERROR("MKO config failed");
    }
    else
    {
        LOG_INFO("tmkconfig() successful!");
    }

    if (bcreset() != 0)
    {
        data += "МКО: Перезагрузка МКО не произошла!\n";
        LOG_ERROR("MKO reset failed");
    }

    mMainKitEnabled = true;
}

void ModuleMKO::stopMKO()
{
    LOG_INFO("Stopping MKO");
    mMainKitEnabled = false; //TODO
    TmkClose();
    CloseHandle(hEvent);
}

void ModuleMKO::stopMKO1()
{
    LOG_INFO("Stopping MKO 1");
    TmkClose();
    CloseHandle(hEvent);
    CloseHandle(hEvent1);
}

void ModuleMKO::processResponseWord(uint16_t responseWord, QStringList& errors)
{
    LOG_INFO(QString("Response word is %1").arg(QString::number(responseWord, 16)));

    //TODO use ADDRESS_MASK etc instead of bit shifts
    uint16_t x;

    uint16_t address;
    if (mMainKitEnabled)
    {
        address = MAIN_KIT_ADDRESS;
    }
    else if (mReserveKitEnabled)
    {
        address = RESERVE_KIT_ADDRESS;
    }
    else
    {
        LOG_ERROR(QString("Response received but no enabled BUP kit found"));
    }

    if (responseWord >> 11 != address)
    {
        LOG_ERROR(QString("Addr from OS: %1, Addr cur: %2").arg(responseWord >> 11).arg(address));
        errors.append(ERR_ADDR_MISMATCH);
    }

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

void ModuleMKO::sendAngleSensorData(uint16_t address)
{
    uint16_t data[3];
    data[0] = 0;
    uint16_t wordsCount = 2;

    if (address == MAIN_KIT_ADDRESS)
    {
        data[1] = PS_FROM_MAIN_KIT;
    }
    else
    {
        data[1] = PS_FROM_RESERVE_KIT;
    }

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

void ModuleMKO::processCommand(const QMap<uint32_t, QVariant>& params)
{
    mCurrentResponse.clear();

    mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);

    uint16_t address = 0;
    if (mMainKitEnabled)
    {
        address = MAIN_KIT_ADDRESS;
    }
    else if (mReserveKitEnabled)
    {
        address = RESERVE_KIT_ADDRESS;
    }
    else
    {
        LOG_ERROR(QString("No MKO Kit enabled"));
        mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(500)); //TODO define error codes internal or hardware
        emit commandResult(mCurrentResponse);
        return;
    }

    ModuleMKO::CommandID command = ModuleMKO::CommandID(params.value(SystemState::COMMAND_ID).toUInt());

    // implicit params check
    switch (command)
    {
    case SEND_TEST_ARRAY_FOR_CHANNEL:
    case RECEIVE_TEST_ARRAY_FOR_CHANNEL:
    case SEND_COMMAND_ARRAY_FOR_CHANNEL:
    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int paramsCount = params.value(SystemState::IMPLICIT_PARAMS_COUNT).toInt();
            if (paramsCount != 1)
            {
                LOG_ERROR(QString("Malformed request for MKO command %1").arg(int(command)));
                mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(50)); //TODO define error codes internal or hardware
                emit commandResult(mCurrentResponse);
                return;
            }

            //case SEND_COMMAND_ARRAY:
            //case SEND_COMMAND_ARRAY_FOR_CHANNEL:
            int TODO; // check input params?
        }
        break;

    default:
        break;
    }

    int errorCode = 0;

    switch (command)
    {
    case SEND_TEST_ARRAY:
        {
            sendTestArray(address);
        }
        break;
    case RECEIVE_TEST_ARRAY:
        {
            receiveTestArray(address);
        }
        break;
    case SEND_COMMAND_ARRAY:
        {
            AxisData psy;
            AxisData nu;

            int paramsCount = params.value(SystemState::INPUT_PARAMS_COUNT).toInt() * 2;
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = params.value(SystemState::INPUT_PARAM_BASE + i).toInt();
                    //qreal vr = params.value(SystemState::INPUT_PARAM_BASE + i + 1).toDouble();
                    //QVariant vart = params.value(SystemState::INPUT_PARAM_BASE + i + 1);
                    int32_t value = int32_t(params.value(SystemState::INPUT_PARAM_BASE + i + 1).toDouble());

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
            }

            sendCommandArray(address, psy, nu);
        }
        break;
    case RECEIVE_COMMAND_ARRAY:
        {
            int paramsCount = params.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
            mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);

            for (int i = 0; i < paramsCount * 2; ++i)
            {
                mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = params.value(SystemState::OUTPUT_PARAM_BASE + i);
            }

            receiveCommandArray(address);
        }
        break;
    case SEND_TEST_ARRAY_FOR_CHANNEL:
        {
            Subaddress subaddress = Subaddress(params.value(SystemState::IMPLICIT_PARAM_BASE + 0).toInt());
            sendTestArrayForChannel(address, subaddress);
        }
        break;
    case RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            int paramsCount = params.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
            mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);

            for (int i = 0; i < paramsCount; ++i)
            {
                mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = params.value(SystemState::OUTPUT_PARAMS_COUNT + i);
            }

            Subaddress subaddress = Subaddress(params.value(SystemState::IMPLICIT_PARAM_BASE + 0).toInt());
            receiveTestArrayForChannel(address, subaddress);
        }
        break;
    case SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            AxisData data;
            int paramsCount = params.value(SystemState::INPUT_PARAMS_COUNT).toInt();
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = params.value(SystemState::INPUT_PARAM_BASE + i).toInt();
                    int32_t value = int32_t(params.value(SystemState::INPUT_PARAM_BASE + i + 1).toDouble());

                    switch (type)
                    {
                    case SystemState::MODE:
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
            }
            Subaddress subaddress = Subaddress(params.value(SystemState::IMPLICIT_PARAM_BASE + 0).toInt());
            sendCommandArrayForChannel(address, subaddress, data);
        }
        break;
    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int paramsCount = params.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
            for (int i = 0; i < paramsCount; ++i)
            {
                mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = params.value(SystemState::OUTPUT_PARAMS_COUNT + i);
            }

            Subaddress subaddress = Subaddress(params.value(SystemState::IMPLICIT_PARAM_BASE + 0).toInt());
            receiveCommandArrayForChannel(address, subaddress);
        }
        break;
    case SEND_TO_ANGLE_SENSOR:
        {
            sendAngleSensorData(address);
        }
        break;

    case START_MKO:
        {
            startMKO();
        }
        break;

    case STOP_MKO:
        {
            stopMKO();
        }
        break;

    default:
        errorCode = 100; //TODO define error code
        break;
    }

    if (errorCode != 0)
    {
        LOG_ERROR(QString("Error in MKO command id=%1").arg(int(command)));
        mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(errorCode)); //TODO define error codes internal or hardware
        emit commandResult(mCurrentResponse);
        return;
    }

    if (mWordsToReceive > RECEIVE_BUFFER_SIZE)
    {
        LOG_ERROR(QString("Receive buffer overflow: Requred size=%1, Available size=%2").arg(mWordsToReceive).arg(RECEIVE_BUFFER_SIZE));
        mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(200)); //TODO define error codes internal or hardware
        emit commandResult(mCurrentResponse);
        return;
    }

    mCurrentResponse[SystemState::MODULE_ID] = params.value(SystemState::MODULE_ID);
    mCurrentResponse[SystemState::COMMAND_ID] = QVariant(uint32_t(command));
    mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(0));

    if (command == START_MKO || command == STOP_MKO)
    {
        emit commandResult(mCurrentResponse);
    }
}

void ModuleMKO::onApplicationStart()
{
    setModuleState(AbstractModule::INITIALIZING);
    setModuleState(AbstractModule::INITIALIZED_OK);
    //startMKO(); //TODO when to stop MKO? and we are using main kit only for now
}

void ModuleMKO::setDefaultState()
{
    //TODO check state
    setModuleState(AbstractModule::SETTING_TO_SAFE_STATE);
    setModuleState(AbstractModule::SAFE_STATE);
    mMainKitEnabled = false;
    mReserveKitEnabled = false;
    int TODO;
}

void ModuleMKO::onCyclogramStart()
{
    startMKO();
}
