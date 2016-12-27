#include "MKO.h"
#include "mainwindow.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QString>
#include "qapplication.h"
#include "synchapi.h"
#include "comport.h"
#include "WDMTMKv2.cpp"

HANDLE hEvent, hEvent1;
int flag_ch = 0;
int addr1, addr2;


MKO::MKO(QString s, QObject * parent) :
    QObject(parent),
    name(s)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(MKO_timer()));
}

void MKO::MKO_chan(int x)
{
    flag_ch = x;
}

void MKO::MKO_avt(int x, int y, int adr1, int adr2)
{
    if(x == 1)
    {
        m_timer->start(y);
        addr1 = adr1;
        addr2 = adr2;
    }
}

void MKO::MKO_timer()
{
    MKO_rc_cm(flag_ch, addr1, addr2);
}

void MKO::startMKO1()
{
    QString data;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hEvent1)
    {
        data += "Ошибка! CreateEvent() не запустился!\n";
    }

    if (!hEvent)
    {
        data += "Ошибка! CreateEvent() не запустился!\n";
    }

    if (TmkOpen())
    {
        data += "Ошибка! Библиотека МКО не запустилась!\n";
    }

    if (tmkconfig(1) != 0 )
    {
        data += "Ошибка! Конфигурация МКО 1 не произошла!\n";
    }

    if (tmkconfig(0) != 0 )
    {
        data += "Ошибка! Конфигурация МКО 0 не произошла!\n";
    }

    if (bcreset() != 0)
    {
        data += "Ошибка! Перезагрузка МКО не произошла!\n";
    }

    emit start_MKO(data);
}

void MKO::startMKO()
{
    QString data;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hEvent)
    {
        data += "Ошибка! CreateEvent() не запустился!\n";
    }

    if (TmkOpen())
    {
        data += "Ошибка! Библиотека МКО не запустилась!\n";
    }

    if (tmkconfig(0) != 0)
    {
        data += "Ошибка! Конфигурация МКО не произошла!\n";
    }

    if (bcreset() != 0)
    {
        data += "Ошибка! Перезагрузка МКО не произошла!\n";
    }

    emit start_MKO(data);
}

void MKO::stopMKO()
{
    TmkClose();
    CloseHandle(hEvent);
}

void MKO::stopMKO1()
{
    TmkClose();
    CloseHandle(hEvent);
    CloseHandle(hEvent1);
}

QString MKO::OCcontrol(WORD oc)
{
    QString data1;
    uint16_t x;
    if (oc >> 11 != m_addr)
    {
        data1 += " Неверный адрес в ОС! \n";
    }

    x = oc << 5;
    x = x >> 15;
    if (x == 1)
    {
        data1 += " Принята недостоверная информация! \n";
    }

    x = oc << 12;
    x = x >> 15;

    if (x == 1)
    {
        data1 += " Нет возможности обмена! \n";
    }

    x = oc << 13;
    x = x >> 15;
    if (x == 1)
    {
        data1 += " Абонент неисправен! \n";
    }

    x = oc << 15;
    x = x >> 15;

    if (x == 1)
    {
        data1 += " ОУ функционирует неправильно! \n";
    }

    if (data1 == "")
    {
        data1 += "Ошибок нет! \n";
    }

    return data1;
}

void MKO::MKO_start_test(int kits, int adr1, int adr2)
{
    WORD buff[13];
    WORD buf[11];
    for(int i = 0; i < 11; i++)
    {
        buf[i] = 0;
    }

    QString err;
    switch (kits)
    {
    case NO_KIT:
        {
            err += " Выберите полукомплект для проверки! \n";
        }
        break;
    case MAIN_KIT:
        {
            err += " Основной полукомплект передача массива: \n";
            m_addr = adr1;
            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 0);
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(2, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = MAIN_KIT;

            m_subAddr = 12;
            err += receive(buf, 11);

            err += " Основной полукомплект прием массива: \n";
            m_addr = adr1;
            send(buff, 12);

            for(int i = 1; i < 12; i++)
            {
                if(buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);
        }
        break;
    case RESERVE_KIT:
        {
            err += " Резервный полукомплект передача массива: \n";
            m_addr = adr2;
            if(m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 0);
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(1, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = RESERVE_KIT;

            m_subAddr = 12;
            err += receive(buf, 11);

            err += " Резервный полукомплект прием массива: \n";
            m_addr = adr2;
            send(buff, 12);

            for(int i = 1; i < 12; i++)
            {
                if(buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);
        }
        break;
    case ALL_KITS:
        {
            if(m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 1);
                Sleep(1200);
                startMKO1();
            }

            if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            m_enabledKits = ALL_KITS;
            if (tmkselect(0) != 0)
            {
                err += "Ошибка! tmk0!\n";
            }

            bcreset();

            m_addr = adr1;
            err += " Основной полукомплект передача массива ось ψ: \n";
            m_subAddr = 1;
            err += receive(buf, 6);

            err += " Основной полукомплект передача массива ось υ: \n";
            m_subAddr = 2;
            err += receive(buf, 6);

            err += " Основной полукомплект прием массива ось ψ: \n";
            m_subAddr = 1;
            send(buff, 7);

            for(int i = 1; i < 7; i++)
            {
                if (buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);

            err += " Основной полукомплект прием массива ось υ: \n";
            m_subAddr = 2;
            send(buff, 7);

            for(int i = 1; i < 7; i++)
            {
                if (buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);

            if (tmkselect(1) != 0 )
            {
                err += "Ошибка! tmk1!\n";
            }

            bcreset();

            m_addr = adr2;
            err += " Резервный полукомплект передача массива ось ψ: \n";
            m_subAddr = 1;
            err += receive(buf, 6);
            err += " Резервный полукомплект передача массива ось υ: \n";
            m_subAddr = 2;
            err += receive(buf, 6);

            m_addr = adr2;
            err += " Резервный полукомплект прием массива ось ψ: \n";
            m_subAddr = 1;
            send(buff, 7);
            for(int i = 1; i < 7; i++)
            {
                if (buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);
            err += " Резервный полукомплект прием массива ось υ: \n";
            m_subAddr = 2;
            send(buff, 7);
            for(int i = 1; i < 7; i++)
            {
                if (buff[i] != 0)
                {
                    err += "Ошибка! Тест провален!";
                }
            }

            err += OCcontrol(buff[0]);
        }
        break;
    default:
        break;
    }

    emit start_MKO(err);
}

void MKO::pow_DY(int kit, int adr)
{
    QString err1 = "Питание ДУ:\n";
    WORD buf[3];
    m_subAddr = 6;
    m_addr = adr;
    buf[0] = 0;

    if (kit == MAIN_KIT)
    {
        if(m_enabledKits == RESERVE_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(m_enabledKits == NO_KIT)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(m_enabledKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        m_enabledKits = MAIN_KIT;
        buf[1] = 32;
        buf[2] = 32;
    }
    else // reserve kit
    {
        if (m_enabledKits == MAIN_KIT)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (m_enabledKits == NO_KIT)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if (m_enabledKits == ALL_KITS)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        m_enabledKits = RESERVE_KIT;
        buf[1] = 64;
        buf[2] = 64;
    }

    err1 += receive(buf, 3);
    emit start_MKO(err1);
}

void MKO::MKO_tr_cm(int kits, QString cm, int adr1, int adr2)
{
    if (kits == NO_KIT)
    {
        emit start_MKO(" Выберите полукомплект для передачи массива! \n");
        return;
    }

    QStringList list1 = cm.split(" ");
    QString err2 = "";

    WORD buf1[11];
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

    for (int i = 0; i < 10; i++)
    {
        buf1[10] = buf1[10] + buf1[i];
    }

    switch (kits)
    {
    case MAIN_KIT:
        {
            err2 += " Основной полукомплект передача массива: \n";
            m_addr = adr1;
            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 0);
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(2, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = MAIN_KIT;
            m_subAddr = 12;
            err2 += receive(buf1, 11);
        }
        break;

    case RESERVE_KIT:
        {
            err2 += " Резервный полукомплект передача массива: \n";
            m_addr = adr2;
            if(m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 0);
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(1, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = RESERVE_KIT;
            m_subAddr = 12;
            err2 += receive(buf1, 11);
        }
        break;

    case ALL_KITS:
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

            if(m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 1);
                Sleep(1200);
                startMKO1();
            }

            if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            m_enabledKits = ALL_KITS;
            if (tmkselect(0) != 0)
            {
                err2 += "Ошибка! tmk0!\n";
            }

            bcreset();

            m_addr = adr1;
            err2 += " Основной полукомплект передача массива ось ψ: \n";
            m_subAddr = 1;
            err2 += receive(buf2, 6);

            err2 += " Основной полукомплект передача массива ось υ: \n";
            m_subAddr = 2;
            err2 += receive(buf3, 6);

            if (tmkselect(1) != 0 )
            {
                err2 += "Ошибка! tmk1!\n";
            }

            bcreset();
            m_addr = adr2;
            err2 += " Резервный полукомплект передача массива ось ψ: \n";
            m_subAddr = 1;
            err2 += receive(buf2, 6);

            err2 += " Резервный полукомплект передача массива ось υ: \n";
            m_subAddr = 2;
            err2 += receive(buf3, 6);
        }
        break;
    default:
        break;
    }

    emit start_MKO(err2);
}

void MKO::MKO_rc_cm(int kits, int adr1, int adr2)
{
    if (kits == NO_KIT)
    {
        emit data_MKO("");
        emit start_MKO(" Выберите полукомплект для приема массива! \n");
        return;
    }

    QString data = "";
    QString err3 = "";

    switch (kits)
    {
    case MAIN_KIT:
        {
            err3 += " Основной полукомплект прием массива: \n";
            m_addr = adr1;
            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 0);
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(2, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = MAIN_KIT;

            WORD buff1[23];
            m_subAddr = 13;
            send(buff1, 22, data, err3);
        }
        break;
    case RESERVE_KIT:
        {
            err3 += " Резервный полукомплект прием массива: \n";
            m_addr = adr2;
            if(m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 0);
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(2, 1);
                Sleep(1100);
                startMKO();
            }
            else if(m_enabledKits == ALL_KITS)
            {
                stopMKO1();
                Sleep(100);
                emit MKO_CTM(1, 0);
                Sleep(1100);
                startMKO();
            }

            m_enabledKits = RESERVE_KIT;

            WORD buff1[23];
            m_subAddr = 13;
            send(buff1, 22, data, err3);
        }
        break;
    case ALL_KITS:
        {
            WORD buff2[23];
            err3 += " Основной полукомплект прием массива: \n";
            m_addr = adr1;
            if (m_enabledKits == MAIN_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            if(m_enabledKits == RESERVE_KIT)
            {
                stopMKO();
                Sleep(100);
                emit MKO_CTM(1, 1);
                Sleep(1200);
                startMKO1();
            }

            if (m_enabledKits == NO_KIT)
            {
                emit MKO_CTM(1, 1);
                emit MKO_CTM(2, 1);
                Sleep(1200);
                startMKO1();
            }

            m_enabledKits = ALL_KITS;
            if (tmkselect(1) != 0)
            {
                err3 += "Ошибка! tmk0!\n";
            }

            bcreset();
            Sleep(100);
            m_subAddr = 13;
            send(buff2, 22, data, err3);

            if (tmkselect(0) != 0)
            {
                err3 += "Ошибка! tmk1!\n";
            }

            bcreset();
            Sleep(100);
            err3 += " Резервный полукомплект прием массива: \n";

            m_addr = adr2;
            m_subAddr = 13;

            send(buff2, 22, data, err3);
        }
        break;
    default:
        break;
    }

    emit data_MKO(data);
    emit start_MKO(err3);
}

void MKO::send(uint16_t* buf, uint16_t len, QString& dat, QString& error)
{
    uint16_t command = (m_addr << 11) + RT_TRANSMIT + (m_subAddr << 5) + ((len - 1) & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, command);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, buf, len);
    error += OCcontrol(buf[0]);
    for (int i = 1; i < len; i++)
    {
        dat += QString::number(buf[i]) + " ";
    }
}

void MKO::send(uint16_t* buf, uint16_t len)
{
    uint16_t command = (m_addr << 11) + RT_TRANSMIT + (m_subAddr << 5) + ((len - 1) & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, command);
    bcstartx(0, DATA_RT_BC | CX_BUS_0 | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, buf, len);
}

QString MKO::receive(uint16_t* buf, uint16_t len)
{
    uint16_t command = (m_addr << 11) + RT_RECEIVE + (m_subAddr << 5) + (len & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, command);
    bcputblk(1, buf, len);
    bcstartx(0, DATA_BC_RT | CX_BUS_0 | CX_STOP | CX_NOSIG);
    Sleep(100);

    WORD buff[13]; // TODO: 13 - x3 why, is it enough? potential stack corruption may occur here
    bcgetblk(0, buff, 1);
    return OCcontrol(buff[0]);
}
