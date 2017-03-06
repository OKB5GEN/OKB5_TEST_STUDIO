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
    static const uint16_t ANGLE_SENSOR_SUBADDRESS = 0x06;
    static const uint16_t PSY_CHANNEL_SUBADDRESS = 0x01;
    static const uint16_t NU_CHANNEL_SUBADDRESS = 0x02;
    static const uint16_t RECEIVE_SUBADDRESS = 0x0D;
    static const uint16_t SEND_SUBADDRESS = 0x0C;

    static const int RECEIVE_DELAY = 100; // msec
    static const int RECEIVE_BUFFER_SIZE = 100; // words
}

ModuleMKO::ModuleMKO(QObject* parent):
    AbstractModule(parent),
    mMainKitEnabled(false),
    mReserveKitEnabled(false),
    mWordsToReceive(0),
    mActiveKits(NO_KIT)
{
    mReceiveTimer = new QTimer(this);
    mReceiveTimer->setSingleShot(true);
    connect(mReceiveTimer, SIGNAL(timeout()), this, SLOT(readResponse()));
}

ModuleMKO::~ModuleMKO()
{

}

void ModuleMKO::readResponse()
{
    int TODO; // стопнули циклограмму, не дождавшись респонса

    ModuleMKO::CommandID command = ModuleMKO::CommandID(mCurrentResponse.value(SystemState::COMMAND_ID).toUInt());

    uint16_t buffer[RECEIVE_BUFFER_SIZE];

    switch (command)
    {
    case SEND_TEST_ARRAY:
        {
            int TODO;
        }
        break;
    case RECEIVE_TEST_ARRAY:
        {
            int TODO;
        }
        break;
    case SEND_COMMAND_ARRAY:
        {
            int TODO;
        }
        break;
    case RECEIVE_COMMAND_ARRAY:
        {
            int TODO;
        }
        break;
    case SEND_TEST_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case SEND_TO_ANGLE_SENSOR:
        {
            int TODO;
        }
        break;
    default:
        LOG_ERROR(QString("MKO messages internal error"));
        break;
    }

    bcgetblk(0, &buffer, mWordsToReceive);

    if (false)
    {
        //TODO check response word
        //TODO read and parse response data
    }

    //TODO можно ли больше прочитать? а меньше? и что будет?

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

    emit start_MKO(data);
}

void ModuleMKO::stopMKO()
{
    TmkClose();
    CloseHandle(hEvent);
}

void ModuleMKO::stopMKO1()
{
    TmkClose();
    CloseHandle(hEvent);
    CloseHandle(hEvent1);
}

QString ModuleMKO::processResponseWord(uint16_t oc)
{
    //ADDRESS_MASK
    QString error;
    uint16_t x;

    uint16_t address;
    if (mMainKitEnabled)
    {
        address == MAIN_KIT_ADDRESS;
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
    uint16_t commandWord = (address << 11) + RT_RECEIVE + (subaddress << 5) + (wordsCount & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, commandWord);
    bcputblk(1, data, wordsCount);
    bcstartx(0, DATA_BC_RT | CX_BUS_0 | CX_STOP | CX_NOSIG);

    mReceiveTimer->start(RECEIVE_DELAY);
}

void ModuleMKO::requestDataFromBUP(uint16_t address, uint16_t subaddress, uint16_t expectedWordsInResponse)
{
    uint16_t commandWord = (address << 11) + RT_TRANSMIT + (subaddress << 5) + ((expectedWordsInResponse - 1) & NWORDS_MASK); //TODO '-1' - "response word" or "checksum"
    bcdefbase(0);
    bcputw(0, commandWord);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);

    mReceiveTimer->start(RECEIVE_DELAY);
}

void ModuleMKO::pow_DY(int kit, int adr)
{
    /*QString err1 = "";
    WORD buf[3];
    WORD buff[5];
    mSubAddr = 6;
    mAddr = adr;
    buf[0] = 0;

    if (kit == NO_KIT)
    {
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
            if (tmkselect(0) != 0)
            {
                err1 += "МКО: Ошибка tmk0!\n";
            }//

            bcreset();
        }

        mActiveKits = MAIN_KIT;
        buf[1] = 32;
        buf[2] = 32;
    }
    else
    {
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
            if (tmkselect(1) != 0)
            {
                err1 += "МКО: Ошибка tmk1!\n";
            }//

            bcreset();
        }

        mActiveKits = RESERVE_KIT;
        buf[1] = 64;
        buf[2] = 64;
    }

    sendDataToBUP(buf, 3, buff, 1);
    if (OCcontrol(buff[0]) != "")
    {
        err1 += "МКО: Питание ДУ:\n";
        err1 += OCcontrol(buff[0]);
    }

    emit start_MKO(err1);*/
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

    ModuleMKO::CommandID command = ModuleMKO::CommandID(params.value(SystemState::COMMAND_ID).toUInt());

    switch (command)
    {
    case SEND_TEST_ARRAY:
        {
            //sendDataToBUP();
            int TODO;
        }
        break;
    case RECEIVE_TEST_ARRAY:
        {
            int TODO;
        }
        break;
    case SEND_COMMAND_ARRAY:
        {
            int TODO;
        }
        break;
    case RECEIVE_COMMAND_ARRAY:
        {
            int TODO;
        }
        break;
    case SEND_TEST_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            int TODO;
        }
        break;
    case SEND_TO_ANGLE_SENSOR:
        {
            int TODO;
        }
        break;

    default:
        {
            LOG_ERROR(QString("Unknown command id=%1").arg(int(command)));
            mCurrentResponse[SystemState::ERROR_CODE] = QVariant(uint32_t(100)); //TODO define error codes internal or hardware
            emit commandResult(mCurrentResponse);
            return;
        }
        break;
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
    setModuleState(AbstractModule::INITIALIZED_FAILED, tr("MKO not implemeted"));

    int TODO;
}

void ModuleMKO::setDefaultState()
{
    //TODO check state
    setModuleState(AbstractModule::SETTING_TO_INACTIVE);
    setModuleState(AbstractModule::SAFE_STATE);
    int TODO;
}
