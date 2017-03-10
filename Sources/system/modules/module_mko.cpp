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
    static const int RECEIVE_BUFFER_SIZE = 100; // words
}

ModuleMKO::ModuleMKO(QObject* parent):
    AbstractModule(parent),
    mMainKitEnabled(false),
    mReserveKitEnabled(false),
    mWordsToReceive(0),
    mWordsSent(0),
    mActiveKits(NO_KIT)
{
    mReceiveTimer = new QTimer(this);
    mReceiveTimer->setSingleShot(true);
    connect(mReceiveTimer, SIGNAL(timeout()), this, SLOT(readResponse()));
}

ModuleMKO::~ModuleMKO()
{
    stopMKO(); // TODO remove
}

void ModuleMKO::readResponse()
{
    int TODO; // стопнули циклограмму, не дождавшись респонса

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

    QString error = processResponseWord(buffer[0]);
    if (!error.isEmpty())
    {
        LOG_ERROR(QString("MKO response error: %1").arg(error));
        mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(305)); //TODO define error codes internal or hardware
    }

    ModuleMKO::CommandID command = ModuleMKO::CommandID(mCurrentResponse.value(SystemState::COMMAND_ID).toUInt());

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
            int paramsCount = mCurrentResponse.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i).toInt();
                    QString variable = mCurrentResponse.value(SystemState::OUTPUT_PARAM_BASE + i + 1).toString();
                    qreal value = 0;

                    switch (type)
                    {
                    case SystemState::MODE_PSY:
                        {
                            int16_t v = buffer[0];
                            value = v;
                        }
                        break;
                    case SystemState::STEPS_PSY:
                        {
                            int16_t v1 = buffer[1];
                            int32_t v = buffer[2] + (v1 << 16);
                            value = v;
                        }
                        break;
                    case SystemState::VELOCITY_PSY:
                        {
                            int16_t v = buffer[3];
                            value = v;
                        }
                        break;
                    case SystemState::CURRENT_PSY:
                        {
                            int16_t v = buffer[4];
                            value = v;
                        }
                        break;
                    case SystemState::ANGLE_PSY:
                        {
                            qreal rawData = buffer[10];
                            value = rawData * 180 / 65536;
                        }
                        break;
                    case SystemState::MODE_NU:
                        {
                            int16_t v = buffer[5];
                            value = v;
                        }
                        break;
                    case SystemState::STEPS_NU:
                        {
                            int16_t v1 = buffer[6];
                            int32_t v = buffer[7] + (v1 << 16);
                            value = v;
                        }
                        break;
                    case SystemState::VELOCITY_NU:
                        {
                            int16_t v = buffer[8];
                            value = v;
                        }
                        break;
                    case SystemState::CURRENT_NU:
                        {
                            int16_t v = buffer[9];
                            value = v;
                        }
                        break;
                    case SystemState::ANGLE_NU:
                        {
                            qreal rawData = buffer[11];
                            value = rawData * 180 / 65536;
                        }
                        break;
                    case SystemState::SENSOR_FLAG:
                        {
                            uint16_t tmp = buffer[19];
                            tmp = tmp << 3;
                            tmp = tmp >> 15;
                            value = tmp;
                        }
                        break;
                    case SystemState::TEMPERATURE:
                        {
                            uint16_t tmp = buffer[19];
                            tmp = tmp << 3;
                            tmp = tmp >> 15;
                            if (tmp == 1) // has temperature sensor
                            {
                                int16_t temper = buffer[19] & 0x0fff; //TODO скорее всего какая-то хуйня, если это интерпретировать как int, а не float
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

                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = QVariant(variable);
                    mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i + 1] = QVariant(value);
                }
            }
        }
        break;

    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int paramsCount = mCurrentResponse.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
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
                            int16_t v = buffer[0];
                            value = v;
                        }
                        break;
                    case SystemState::STEPS:
                        {
                            int16_t v1 = buffer[1];
                            int32_t v = buffer[2] + (v1 << 16);
                            value = v;
                        }
                        break;
                    case SystemState::VELOCITY:
                        {
                            int16_t v = buffer[3];
                            value = v;
                        }
                        break;
                    case SystemState::CURRENT:
                        {
                            int16_t v = buffer[4];
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
        }
        break;

    default:
        LOG_ERROR(QString("MKO messages internal error"));
        break;
    }

    emit commandResult(mCurrentResponse);
}

void ModuleMKO::MKO_timer()
{
    //MKO_rc_cm(flag_ch, addr1,  addr2);
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

    emit start_MKO(data);
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
    //emit start_MKO(data);
}

void ModuleMKO::stopMKO()
{
    LOG_INFO("Stopping MKO");
    mMainKitEnabled = false;
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

QString ModuleMKO::processResponseWord(uint16_t oc)
{
    LOG_INFO(QString("Response word is %1").arg(QString::number(oc, 16)));

    //TODO use ADDRESS_MASK etc instead of bit shifts
    QString error;
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

    if (oc >> 11 != address)
    {
        LOG_ERROR(QString("Addr from OS: %1, Addr cur: %2").arg(oc >> 11).arg(address));
        error += " - Неверный адрес в ОС! \n";
    }

    x = oc << 5;
    x = x >> 15;
    if (x == 1)
    {
        error += " - Принята недостоверная информация! \n";
    }

    x = oc << 12;
    x = x >> 15;

    if (x == 1)
    {
        error += " - Нет возможности обмена! \n";
    }

    x = oc << 13;
    x = x >> 15;

    if (x == 1)
    {
        error += " - Абонент неисправен! \n";
    }

    x = oc << 15;
    x = x >> 15;

    if (x == 1)
    {
        error += " - ОУ функционирует неправильно! \n";
    }

    if (error == "")
    {
        error += "";
    }

    return error;
}

void ModuleMKO::MKO_start_test(int kit, int adr1, int adr2)
{
    /*WORD buf[11];
    WORD buff[13];

    QString err = "";
    int err_count = 0;
    if (kit == NO_KIT)
    {
        err += "МКО: Выберите полукомплект для проверки! \n";
    }

    for(int i = 0; i < 11; i++)
    {
        buf[i] = 0;
    }

    if (kit == MAIN_KIT)
    {
        err += "МКО: Основной полукомплект передача массива: \n";
        mAddr = adr1;

        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = MAIN_KIT;
    }
    else if (kit == RESERVE_KIT)
    {
        err += "МКО: Резервный полукомплект передача массива: \n";
        mAddr = adr2;
        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = RESERVE_KIT;
    }

    if (kit == MAIN_KIT || kit == RESERVE_KIT)
    {
        mSubAddr = 12;
        sendDataToBUP(buf, 11, buff, 1);
        err += OCcontrol(buff[0]);
    }

    if (kit == MAIN_KIT)
    {
        err += "МКО: Основной полукомплект прием массива: \n";
        mAddr = adr1;
    }
    else if (kit == RESERVE_KIT)
    {
        err += "МКО: Резервный полукомплект прием массива: \n";
        mAddr = adr2;
    }

    if (kit == MAIN_KIT || kit == RESERVE_KIT)
    {
        mSubAddr = 12;
        requestDataFromBUP(buff, 12);
        for(int i = 1; i < 12; i++)
        {
            if (buff[i] != 0)
            {
                err_count++;
            }
        }

        if (err_count != 0)
        {
            err += "МКО: Тест провален!\n";
        }

        if (OCcontrol(buff[0]) != "")
        {
            err += OCcontrol(buff[0]);
        }

        else if (OCcontrol(buff[0]) != "" && err_count == 0)
        {
            err = "";
        }

        err_count = 0;
    }

    if (kit == ALL_KITS)
    {
        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        mActiveKits = ALL_KITS;

        if (tmkselect(0) != 0)
        {
            err+= "МКО: Ошибка tmk0!\n";
        }//

        bcreset();

        mAddr = adr1;
        mSubAddr = 1;

        sendDataToBUP(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Основной полукомплект передача массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        sendDataToBUP(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Основной полукомплект передача массива ось υ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 1;
        requestDataFromBUP(buff, 7);
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err_count++;
            }
        }

        if (err_count != 0)
        {
            err+= "МКО: Основной полукомплект прием массива ось ψ: \n";
            err+= "МКО: Тест провален!\n";
        }

        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Основной полукомплект прием массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        err_count = 0;
        mSubAddr = 2;
        requestDataFromBUP(buff, 7);
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err_count++;
            }
        }

        if (err_count != 0)
        {
            err+= "МКО: Основной полукомплект прием массива ось υ: \n";
            err+= "МКО: Тест провален!\n";
        }

        if (OCcontrol(buff[0]) != "")
        {
            err+= "МКО: Основной полукомплект прием массива ось υ: \n";
            err+= OCcontrol(buff[0]);
        }

        err_count = 0;
        if (tmkselect(1) != 0)
        {
            err+= "МКО: Ошибка tmk1!\n";
        }//

        bcreset();

        //ch_cur = 2;
        mAddr = adr2;
        mSubAddr = 1;
        sendDataToBUP(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект передача массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        sendDataToBUP(buf, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект передача массива ось υ: \n";
            err += OCcontrol(buff[0]);
        }

        mAddr = adr2;
        mSubAddr = 1;
        requestDataFromBUP(buff, 7);
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err_count++;
            }
        }

        if (err_count != 0)
        {
            err += "МКО: Резервный полукомплект прием массива ось ψ: \n";
            err += "МКО: Тест провален!\n";
        }

        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект прием массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        err_count= 0;
        mSubAddr = 2;
        requestDataFromBUP(buff, 7);

        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err_count++;
            }
        }

        if (err_count != 0)
        {
            err += "МКО: Резервный полукомплект прием массива ось υ: \n";
            err += "МКО: Тест провален!\n";
        }

        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект прием массива ось υ: \n";
            err += OCcontrol(buff[0]);
        }

        err_count= 0;
    }

    emit start_MKO(err);*/
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

    mReceiveTimer->start(RECEIVE_DELAY);
}

void ModuleMKO::requestDataFromBUP(uint16_t address, uint16_t subaddress, uint16_t expectedWordsInResponse)
{
    LOG_INFO(QString("MKO: Request data from BUP: Address: %1, Subaddress: %2").arg(address).arg(subaddress));

    uint16_t commandWord = (address << 11) + RT_TRANSMIT + (subaddress << 5) + ((expectedWordsInResponse - 1) & NWORDS_MASK); //TODO '-1' - "response word" or "checksum"
    bcdefbase(0);
    bcputw(0, commandWord);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);

    mReceiveTimer->start(RECEIVE_DELAY);
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
    data[6] = psy.steps >> 16;
    data[7] = psy.steps;
    data[8] = psy.velocity;
    data[9] = psy.current;

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

void ModuleMKO::MKO_tr_cm(int kit, QString cm, int adr1, int adr2)
{
    /*WORD buf1[11];
    WORD buff[13];

    QStringList list1 = cm.split(" ");
    QString err2 = "";

    if (kit >= 10) //WTF?
    {
        kit -= 10;
        bool ok;
        buf1[0] = list1[0].toInt(&ok, 16);
        buf1[1] = list1[1].toInt(&ok, 16) >> 16;
        buf1[2] = list1[1].toInt(&ok, 16);
        buf1[3] = list1[2].toInt(&ok, 16);
        buf1[4] = list1[3].toInt(&ok, 16);
        buf1[5] = list1[4].toInt(&ok, 16);
        buf1[6] = list1[5].toInt(&ok, 16) >> 16;
        buf1[7] = list1[5].toInt(&ok, 16);
        buf1[8] = list1[6].toInt(&ok, 16);
        buf1[9] = list1[7].toInt(&ok, 16);
        buf1[10] = 0;
    }
    else
    {
        buf1[0] = list1[0].toInt();
        buf1[1] = list1[1].toInt() >> 16;
        buf1[2] = list1[1].toInt();
        buf1[3] = list1[2].toInt();
        buf1[4] = list1[3].toInt();
        buf1[5] = list1[4].toInt();
        buf1[6] = list1[5].toInt() >> 16;
        buf1[7] = list1[5].toInt();
        buf1[8] = list1[6].toInt();
        buf1[9] = list1[7].toInt();
        buf1[10] = 0;
    }

    if (kit == NO_KIT)
    {
        err2 += "МКО: Выберите полукомплект для передачи массива! \n";
    }

    for (int i = 0; i < 10; i++)
    {
        buf1[10] = buf1[10] + buf1[i];
    }

    if (kit == MAIN_KIT)
    {
        err2 += "МКО: Основной полукомплект передача массива: \n";
        mAddr = adr1;
        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = MAIN_KIT;
    }
    else if (kit == RESERVE_KIT)
    {
        err2 += "МКО: Резервный полукомплект передача массива: \n";
        mAddr = adr2;
        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = RESERVE_KIT;
    }

    if (kit == MAIN_KIT || kit == RESERVE_KIT)
    {
        mSubAddr = 12;
        sendDataToBUP(buf1, 11, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += OCcontrol(buff[0]);
        }
        else
        {
            err2 = "";
        }
    }

    if (kit == ALL_KITS)
    {
        WORD buf2[6];
        WORD buf3[6];
        buf2[0] = list1[0].toInt();
        buf2[1] = list1[1].toInt() >> 16;
        buf2[2] = list1[1].toInt();
        buf2[3] = list1[2].toInt();
        buf2[4] = list1[3].toInt();
        buf2[5] = 0;
        for (int j = 0; j < 5; j++)
        {
            buf2[5] = buf2[5] + buf2[j];
        }

        buf3[0] = list1[4].toInt();
        buf3[1] = list1[5].toInt() >> 16;
        buf3[2] = list1[5].toInt();
        buf3[3] = list1[6].toInt();
        buf3[4] = list1[7].toInt();
        buf3[5] = 0;
        for (int k = 0; k < 5; k++)
        {
            buf3[5] = buf3[5] + buf3[k];
        }

        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        mActiveKits = ALL_KITS;

        //bcreset();
        if (tmkselect(0) != 0)
        {
            err2 += "Ошибка! tmk0!\n";
        }//

        bcreset();

        mAddr = adr1;
        mSubAddr = 1;
        sendDataToBUP(buf2, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Основной полукомплект передача массива ось ψ: \n";
            err2 += OCcontrol(buff[0]);
        }

        mSubAddr = 2;

        sendDataToBUP(buf3, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Основной полукомплект передача массива ось υ: \n";
            err2 += OCcontrol(buff[0]);
        }

        //bcreset();
        if (tmkselect(1) != 0)
        {
            err2 += "МКО: Ошибка! tmk1!\n";
        }//

        bcreset();
        mAddr = adr2;
        mSubAddr = 1;

        sendDataToBUP(buf2, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Резервный полукомплект передача массива ось ψ: \n";
            err2 += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        sendDataToBUP(buf3, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Резервный полукомплект передача массива ось υ: \n";
            err2 += OCcontrol(buff[0]);
        }
    }

    emit start_MKO(err2);*/
}

void ModuleMKO::MKO_rc_cm(int kit, int adr1, int adr2)
{
    /*QString data = "";
    QString err3 = "";
    if (kit == NO_KIT)
    {
        err3 += "МКО: Выберите полукомплект для приема массива! \n";
    }

    if (kit == MAIN_KIT)
    {
        err3 += "МКО: Основной полукомплект прием массива: \n";
        mAddr = adr1;
        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = MAIN_KIT;
    }

    if (kit == RESERVE_KIT)
    {
        err3 += "МКО: Резервный полукомплект прием массива: \n";
        mAddr = adr2;
        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (mActiveKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        mActiveKits = RESERVE_KIT;
    }

    if (kit == MAIN_KIT || kit == RESERVE_KIT)
    {
        mSubAddr = 13;
        WORD buff1[23];
        requestDataFromBUP(buff1, 22);

        if (OCcontrol(buff1[0]) != "")
        {
            err3 += OCcontrol(buff1[0]);
        }
        else
        {
            err3 = "";
        }

        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff1[i]) + " ";
        }
    }

    if (kit == ALL_KITS)
    {
        mAddr = adr1;
        if (mActiveKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (mActiveKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        mActiveKits = ALL_KITS;
        if (tmkselect(1) != 0)
        {
            err3 += "МКО: Ошибка tmk0!\n";
        }//

        bcreset();
        Sleep(100);
        mSubAddr = 13;

        WORD buff2[23];

        requestDataFromBUP(buff2, 22);
        if (OCcontrol(buff2[0]) != "")
        {
            err3 += "МКО: Основной полукомплект прием массива: \n";
            err3 += OCcontrol(buff2[0]);
        }

        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff2[i]) + " ";
        }

        if (tmkselect(0) != 0)
        {
            err3 += "МКО: Ошибка tmk1!\n";
        }//
        bcreset();

        Sleep(100);
        mAddr = adr2;
        mSubAddr = 13;

        requestDataFromBUP(buff2, 22);

        if (OCcontrol(buff2[0]) != "")
        {
            err3 += "МКО: Резервный полукомплект прием массива: \n";
            err3 += OCcontrol(buff2[0]);
        }

        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff2[i]) + " ";
        }
    }

    emit data_MKO(data);
    emit start_MKO(err3);*/
}

void ModuleMKO::processCommand(const QMap<uint32_t, QVariant>& params)
{
    mCurrentResponse.clear();

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

            int paramsCount = params.value(SystemState::INPUT_PARAMS_COUNT).toInt();
            for (int i = 0; i < paramsCount; ++i)
            {
                if (i % 2 == 0)
                {
                    int type = params.value(SystemState::INPUT_PARAM_BASE + i).toInt();
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

            for (int i = 0; i < paramsCount; ++i)
            {
                mCurrentResponse[SystemState::OUTPUT_PARAM_BASE + i] = params.value(SystemState::OUTPUT_PARAMS_COUNT + i);
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
    mCurrentResponse[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(0);
}

void ModuleMKO::onApplicationStart()
{
    setModuleState(AbstractModule::INITIALIZING);
    startMKO(); //TODO when to stop MKO? and we are using main kit only for now
    setModuleState(AbstractModule::INITIALIZED_OK);
}

void ModuleMKO::setDefaultState()
{
    //TODO check state
    setModuleState(AbstractModule::SETTING_TO_SAFE_STATE);
    setModuleState(AbstractModule::SAFE_STATE);
    int TODO;
}
