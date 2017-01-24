#include "Headers/system/modules/module_mko.h"
#include <windows.h>
#include <QTimer>
#include <QTime>

#include "Sources/system/WDMTMKv2.cpp"
#include "Headers/logger/Logger.h"

HANDLE hEvent, hEvent1;
int flag_ch = 0;
int ch_cur = 0;
int addr1;
int addr2;
//QTimer *MyTimerMKO;

ModuleMKO::ModuleMKO(QObject* parent):
    QObject(parent)
{
//    m_timer = new QTimer(this);
//    m_timer->setSingleShot(true);
//    connect(m_timer, SIGNAL(timeout()), this, SLOT(MKO_timer()));
}

ModuleMKO::~ModuleMKO()
{

}

void ModuleMKO::MKO_chan(int x)
{
    flag_ch = x;
    if (flag_ch == 10 || flag_ch == 20)
    {
        stopMKO();
        flag_ch = 0;
        ch_cur = 0;
    }

    if (flag_ch == 30)
    {
        stopMKO1();
        flag_ch = 0;
        ch_cur = 0;
    }
}

void ModuleMKO::MKO_avt(int x, int y, int adr1, int adr2)
{
    /*if (x == 1)
    {
        MyTimerMKO = new QTimer;
        MyTimerMKO->start(y);
        addr1 = adr1;
        addr2 = adr2;
        QObject::connect(MyTimerMKO,SIGNAL(timeout()), this, SLOT(MKO_timer()));
    }
    else
    {
        QObject::disconnect(MyTimerMKO,SIGNAL(timeout()), this, SLOT(MKO_timer()));
    }*/
}

void ModuleMKO::MKO_timer()
{
    MKO_rc_cm(flag_ch, addr1,  addr2);
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

QString ModuleMKO::OCcontrol(uint16_t oc)
{
    QString data1;
    uint16_t x;
    if (oc >> 11 != mAddr)
    {
        data1+= " - Неверный адрес в ОС! \n";
    }

    x = oc << 5;
    x = x >> 15;
    if (x == 1)
    {
        data1 += " - Принята недостоверная информация! \n";
    }

    x = oc << 12;
    x = x >> 15;

    if (x == 1)
    {
        data1 += " - Нет возможности обмена! \n";
    }

    x = oc << 13;
    x = x >> 15;

    if (x == 1)
    {
        data1 += " - Абонент неисправен! \n";
    }

    x = oc << 15;
    x = x >> 15;

    if (x == 1)
    {
        data1+= " - ОУ функционирует неправильно! \n";
    }

    if (data1 == "")
    {
        data1 += "";
    }

    return data1;
}

void ModuleMKO::MKO_start_test(int x, int adr1, int adr2)
{
    WORD buf[11];
    WORD buff[13];

    QString err = "";
    int err_count = 0;
    if (x == 0)
    {
        err += "МКО: Выберите полукомплект для проверки! \n";
    }

    for(int i = 0; i < 11; i++)
    {
        buf[i] = 0;
    }

    if (x == 1)
    {
        err += "МКО: Основной полукомплект передача массива: \n";
        mAddr = adr1;
        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 1;
    }
    else if (x == 2)
    {
        err += "МКО: Резервный полукомплект передача массива: \n";
        mAddr = adr2;
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 2;
    }

    if (x == 1 || x == 2)
    {
        mSubAddr = 12;
        tx_mes(buf, 11, buff, 1);
        err += OCcontrol(buff[0]);
    }

    if (x == 1)
    {
        err += "МКО: Основной полукомплект прием массива: \n";
        mAddr = adr1;
    }
    else if (x == 2)
    {
        err += "МКО: Резервный полукомплект прием массива: \n";
        mAddr = adr2;
    }

    if (x == 1 || x == 2)
    {
        mSubAddr = 12;
        rx_mes(buff, 12);
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

    if (x == 3)
    {
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        ch_cur = 3;
        if (tmkselect(0) != 0)
        {
            err+= "МКО: Ошибка tmk0!\n";
        }//

        bcreset();

        mAddr = adr1;
        mSubAddr = 1;

        tx_mes(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Основной полукомплект передача массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        tx_mes(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Основной полукомплект передача массива ось υ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 1;
        rx_mes(buff, 7);
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
        rx_mes(buff, 7);
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
        tx_mes(buf, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект передача массива ось ψ: \n";
            err += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        tx_mes(buf, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err += "МКО: Резервный полукомплект передача массива ось υ: \n";
            err += OCcontrol(buff[0]);
        }

        mAddr = adr2;
        mSubAddr = 1;
        rx_mes(buff, 7);
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
        rx_mes(buff, 7);

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

    emit start_MKO(err);
}

void ModuleMKO::tx_mes(uint16_t* sendBuffer, uint16_t sendCount, uint16_t* receiveBuffer, uint16_t receiveCount)
{
    WORD msgCmd = (mAddr << 11) + RT_RECEIVE + (mSubAddr << 5) + (sendCount & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, msgCmd);
    bcputblk(1, sendBuffer, sendCount);
    bcstartx(0, DATA_BC_RT | CX_BUS_0 | CX_STOP | CX_NOSIG);
    Sleep(100);

    bcgetblk(0, receiveBuffer, receiveCount);
}

void ModuleMKO::rx_mes(uint16_t* receiveBuffer, uint16_t receiveCount)
{
    WORD msgCmd = (mAddr << 11) + RT_TRANSMIT + (mSubAddr << 5) + ((receiveCount - 1) & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, msgCmd);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, receiveBuffer, receiveCount);
}

void ModuleMKO::pow_DY(int x, int adr)
{
    QString err1 = "";
    WORD buf[3];
    WORD buff[5];
    mSubAddr = 6;
    mAddr = adr;
    buf[0] = 0;

    if (x == 0)
    {
        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            if (tmkselect(0) != 0)
            {
                err1 += "МКО: Ошибка tmk0!\n";
            }//

            bcreset();
        }

        ch_cur = 1;
        buf[1] = 32;
        buf[2] = 32;
    }
    else
    {
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            if (tmkselect(1) != 0)
            {
                err1 += "МКО: Ошибка tmk1!\n";
            }//

            bcreset();
        }

        ch_cur = 2;
        buf[1] = 64;
        buf[2] = 64;
    }

    tx_mes(buf, 3, buff, 1);
    if (OCcontrol(buff[0]) != "")
    {
        err1 += "МКО: Питание ДУ:\n";
        err1 += OCcontrol(buff[0]);
    }

    emit start_MKO(err1);
}

void ModuleMKO::MKO_tr_cm(int x, QString cm, int adr1, int adr2)
{
    WORD buf1[11];
    WORD buff[13];

    QStringList list1 = cm.split(" ");
    QString err2 = "";

    if (x >= 10)
    {
        x = x - 10;
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

    if (x == 0)
    {
        err2 += "МКО: Выберите полукомплект для передачи массива! \n";
    }

    for (int i = 0; i < 10; i++)
    {
        buf1[10] = buf1[10] + buf1[i];
    }

    if (x == 1)
    {
        err2 += "МКО: Основной полукомплект передача массива: \n";
        mAddr = adr1;
        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 1;
    }
    else if (x == 2)
    {
        err2 += "МКО: Резервный полукомплект передача массива: \n";
        mAddr = adr2;
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 2;
    }

    if (x == 1 || x == 2)
    {
        mSubAddr = 12;
        tx_mes(buf1, 11, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += OCcontrol(buff[0]);
        }
        else
        {
            err2 = "";
        }
    }

    if (x == 3)
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

        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        ch_cur = 3;
        //bcreset();
        if (tmkselect(0) != 0)
        {
            err2+= "Ошибка! tmk0!\n";
        }//

        bcreset();

        mAddr = adr1;
        mSubAddr = 1;
        tx_mes(buf2, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Основной полукомплект передача массива ось ψ: \n";
            err2 += OCcontrol(buff[0]);
        }

        mSubAddr = 2;

        tx_mes(buf3, 6, buff, 1);
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

        tx_mes(buf2, 6, buff, 1);
        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Резервный полукомплект передача массива ось ψ: \n";
            err2 += OCcontrol(buff[0]);
        }

        mSubAddr = 2;
        tx_mes(buf3, 6, buff, 1);

        if (OCcontrol(buff[0]) != "")
        {
            err2 += "МКО: Резервный полукомплект передача массива ось υ: \n";
            err2 += OCcontrol(buff[0]);
        }
    }

    emit start_MKO(err2);
}

void ModuleMKO::MKO_rc_cm(int x, int adr1, int adr2)
{
    QString data = "";
    QString err3 = "";
    if (x == 0)
    {
        err3 += "МКО: Выберите полукомплект для приема массива! \n";
    }

    if (x == 1)
    {
        err3 += "МКО: Основной полукомплект прием массива: \n";
        mAddr = adr1;
        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 1;
    }

    if (x == 2)
    {
        err3 += "МКО: Резервный полукомплект прием массива: \n";
        mAddr = adr2;
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 2;
    }

    if (x == 1 || x == 2)
    {
        mSubAddr = 13;
        WORD buff1[23];
        rx_mes(buff1, 22);

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

    if (x == 3)
    {
        mAddr = adr1;
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if (ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        ch_cur = 3;
        if (tmkselect(1) != 0)
        {
            err3 += "МКО: Ошибка tmk0!\n";
        }//

        bcreset();
        Sleep(100);
        mSubAddr = 13;

        WORD buff2[23];

        rx_mes(buff2, 22);
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

        rx_mes(buff2, 22);

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
    emit start_MKO(err3);
}
