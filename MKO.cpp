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
#include "WDMTMKv2.cpp" // This is some kind of shit to include cpp files

HANDLE hEvent, hEvent1;
int hTmk,flag_ch,ch_cur=0;
unsigned short wBase, wAddr, wMaxBase;
unsigned short awBuf[64];
byte AddrOU,SubAddrOU;
int addr1, addr2;
WORD buf[11];
WORD buff[13];
WORD buff1[23];
WORD msgCmd;
WORD bus;

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
    MKO_rc_cm( flag_ch, addr1,  addr2);
}

void MKO::startMKO1()
{
    QString data;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hEvent1=CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hEvent1)
    {
        data += "Ошибка! CreateEvent() не запустился!\n";
        printf("CreateEvent() failed!\n");
    }

    if (!hEvent)
    {
        data += "Ошибка! CreateEvent() не запустился!\n";
        printf("CreateEvent() failed!\n");
    }

    if (TmkOpen())
    {
        data += "Ошибка! Библиотека МКО не запустилась!\n";
        printf("TmkOpen() failed!\n");
    }
    else
    {
        printf("TmkOpen() successful!\n");
    }

    if (tmkconfig(1) != 0 )
    {
        data += "Ошибка! Конфигурация МКО 1 не произошла!\n";
    }
    else
    {
        printf("tmkconfig1() successful!\n");
    }

    if (tmkconfig(0) != 0 )
    {
        data += "Ошибка! Конфигурация МКО 0 не произошла!\n";
    }
    else
    {
        printf("tmkconfig0() successful!\n");
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
        printf("CreateEvent() failed!\n");
    }

    if (TmkOpen())
    {
        data += "Ошибка! Библиотека МКО не запустилась!\n";
        printf("TmkOpen() failed!\n");
    }
    else
    {
        printf("TmkOpen() successful!\n");
    }

    if (tmkconfig(0) != 0)
    {
        data += "Ошибка! Конфигурация МКО не произошла!\n";
    }
    else
    {
        printf("tmkconfig() successful!\n");
    }

    if (bcreset()!=0)
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
    if (oc >> 11 != AddrOU)
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

void MKO::MKO_start_test(int x, int adr1, int adr2)
{
    QString err = "";
    if (x == 0)
    {
        err += " Выберите полукомплект для проверки! \n";
    }

    for(int i = 0; i < 11; i++)
    {
        buf[i] = 0;
    }

    if (x == 1)
    {
        err += " Основной полукомплект передача массива: \n";
        AddrOU = adr1;
        if(ch_cur != 1 && ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2,0);
            emit MKO_CTM(1,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(1,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2,0);
            Sleep(1100);
            startMKO();
        }

        ch_cur=1;
    }
    else if (x == 2)
    {
        err += " Резервный полукомплект передача массива: \n";
        AddrOU = adr2;
        if(ch_cur != 2 && ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1,0);
            emit MKO_CTM(2,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(2,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1,0);
            Sleep(1100);
            startMKO();
        }

        ch_cur=2;
    }

    if (x == 1 || x == 2)
    {
        SubAddrOU = 12;
        msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (11 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf, 11);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err += OCcontrol(buff[0]);
    }

    if (x == 1)
    {
        err += " Основной полукомплект прием массива: \n";
        AddrOU = adr1;
    }
    else if (x == 2)
    {
        err += " Резервный полукомплект прием массива: \n";
        AddrOU = adr2;
    }

    if (x == 1 || x == 2)
    {
        SubAddrOU = 12;
        msgCmd = (AddrOU << 11) + RT_TRANSMIT + (SubAddrOU << 5) + (11 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bus = CX_BUS_0;
        bcstartx(0, DATA_RT_BC | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 12);
        for(int i = 1; i < 12; i++)
        {
            if(buff[i] != 0)
            {
                err += "Ошибка! Тест провален!";
            }
        }

        err += OCcontrol(buff[0]);
    }

    if(x == 3)
    {
        if(ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if(ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if(ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        ch_cur = 3;
        if (tmkselect(0) != 0)
        {
            err += "Ошибка! tmk0!\n";
        }

        bcreset();

        AddrOU = adr1;
        err += " Основной полукомплект передача массива ось ψ: \n";
        SubAddrOU = 1;
        tx_mes();
        err += OCcontrol(buff[0]);
        err += " Основной полукомплект передача массива ось υ: \n";
        SubAddrOU = 2;
        tx_mes();
        err += OCcontrol(buff[0]);

        err += " Основной полукомплект прием массива ось ψ: \n";
        SubAddrOU = 1;
        rx_mes();
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err += "Ошибка! Тест провален!";
            }
        }

        err += OCcontrol(buff[0]);
        err += " Основной полукомплект прием массива ось υ: \n";
        SubAddrOU = 2;
        rx_mes();

        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err += "Ошибка! Тест провален!";
            }
        }

        err += OCcontrol(buff[0]);

        if (tmkselect(1)!= 0 )
        {
            err += "Ошибка! tmk1!\n";
        }

        bcreset();

        AddrOU = adr2;
        err += " Резервный полукомплект передача массива ось ψ: \n";
        SubAddrOU = 1;
        tx_mes();
        err += OCcontrol(buff[0]);
        err += " Резервный полукомплект передача массива ось υ: \n";
        SubAddrOU = 2;
        tx_mes();
        err += OCcontrol(buff[0]);

        AddrOU = adr2;
        err += " Резервный полукомплект прием массива ось ψ: \n";
        SubAddrOU = 1;
        rx_mes();
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err += "Ошибка! Тест провален!";
            }
        }

        err += OCcontrol(buff[0]);
        err += " Резервный полукомплект прием массива ось υ: \n";
        SubAddrOU = 2;
        rx_mes();
        for(int i = 1; i < 7; i++)
        {
            if (buff[i] != 0)
            {
                err += "Ошибка! Тест провален!";
            }
        }

        err += OCcontrol(buff[0]);
    }

    emit start_MKO(err);
}

void MKO::tx_mes()
{
    msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (6 & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, msgCmd);
    bcputblk(1, buf, 6);
    bus = CX_BUS_0;
    bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, buff, 1);
}

void MKO::rx_mes()
{
    msgCmd = (AddrOU << 11) + RT_TRANSMIT + (SubAddrOU << 5) + (6 & NWORDS_MASK);
    bcdefbase(0);
    bcputw(0, msgCmd);
    bus = CX_BUS_0;
    bcstartx(0, DATA_RT_BC | bus | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, buff, 7);
}

void MKO::pow_DY(int x, int adr)
{
    QString err1="";
    err1 += "Питание ДУ:\n";
    WORD buf[3];
    WORD buff[5];
    SubAddrOU = 6;
    AddrOU = adr;
    WORD msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (3 & NWORDS_MASK);
    buf[0] = 0;
    if (x == 0)
    {
        if(ch_cur != 1 && ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(1,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur = 1;
        buf[1] = 32;
        buf[2] = 32;
    }
    else
    {
        if (ch_cur != 2 && ch_cur == 1)
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
        buf[1] = 64;
        buf[2] = 64;
    }

    bcdefbase(0);
    bcputw(0, msgCmd);
    bcputblk(1, buf, 3);
    WORD bus = CX_BUS_0;
    bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
    Sleep(100);
    bcgetblk(0, buff, 1);
    err1 += OCcontrol(buff[0]);
    emit start_MKO(err1);
}

void MKO::MKO_tr_cm(int x, QString cm, int adr1, int adr2)
{
    QStringList list1 = cm.split(" ");
    QString err2 = "";
    if (x == 0)
    {
        err2 += " Выберите полукомплект для передачи массива! \n";
    }

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

    if (x == 1)
    {
        err2 += " Основной полукомплект передача массива: \n";
        AddrOU = adr1;
        if(ch_cur != 1 && ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2,0);
            emit MKO_CTM(1,1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(2, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur=1;
    }
    else if (x == 2)
    {
        err2 += " Резервный полукомплект передача массива: \n";
        AddrOU = adr2;
        if(ch_cur != 2 && ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur=2;
    }

    if (x == 1 || x == 2)
    {
        SubAddrOU = 12;
        msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (11 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf1, 11);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err2 += OCcontrol(buff[0]);
    }

    if(x == 3)
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

        if(ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if(ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 1);
            Sleep(1200);
            startMKO1();
        }

        if(ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        ch_cur = 3;
        if (tmkselect(0) != 0)
        {
            err2 += "Ошибка! tmk0!\n";
        }

        bcreset();

        AddrOU = adr1;
        err2 += " Основной полукомплект передача массива ось ψ: \n";
        SubAddrOU = 1;
        msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (6 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf2, 6);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err2 += OCcontrol(buff[0]);

        err2 += " Основной полукомплект передача массива ось υ: \n";
        SubAddrOU = 2;
        msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (6 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf3, 6);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err2 += OCcontrol(buff[0]);

        if (tmkselect(1) != 0 )
        {
            err2 += "Ошибка! tmk1!\n";
        }

        bcreset();
        AddrOU = adr2;
        err2 += " Резервный полукомплект передача массива ось ψ: \n";
        SubAddrOU = 1;
        msgCmd = (AddrOU << 11) + RT_RECEIVE + (SubAddrOU << 5) + (6 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf2, 6);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err2 += OCcontrol(buff[0]);

        err2 += " Резервный полукомплект передача массива ось υ: \n";
        SubAddrOU = 2;
        msgCmd = (AddrOU << 11) + RT_RECEIVE +(SubAddrOU << 5) + (6 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bcputblk(1, buf3, 6);
        bus = CX_BUS_0;
        bcstartx(0, DATA_BC_RT | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff, 1);
        err2 += OCcontrol(buff[0]);

    }

    emit start_MKO(err2);
}

void MKO::MKO_rc_cm(int x, int adr1, int adr2)
{
    QString data = "";
    QString err3 = "";
    if (x == 0)
    {
        err3 += " Выберите полукомплект для приема массива! \n";
    }

    if (x == 1)
    {
        err3 += " Основной полукомплект прием массива: \n";
        AddrOU = adr1;
        if(ch_cur != 1 && ch_cur == 2)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 0);
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(1, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
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
        err3 += " Резервный полукомплект прием массива: \n";
        AddrOU = adr2;
        if(ch_cur != 2 && ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(1, 0);
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 0)
        {
            emit MKO_CTM(2, 1);
            Sleep(1100);
            startMKO();
        }
        else if(ch_cur == 3)
        {
            stopMKO1();
            Sleep(100);
            emit MKO_CTM(1, 0);
            Sleep(1100);
            startMKO();
        }

        ch_cur=2;
    }

    if (x == 1 || x == 2)
    {
        SubAddrOU = 13;
        msgCmd = (AddrOU << 11) + RT_TRANSMIT + (SubAddrOU << 5) + (21 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bus = CX_BUS_0;
        bcstartx(0, DATA_RT_BC | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff1, 22);
        err3 += OCcontrol(buff1[0]);
        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff1[i]) + " ";
        }
    }

    if (x == 3)
    {
        WORD buff2[23];
        err3 += " Основной полукомплект прием массива: \n";
        AddrOU = adr1;
        if (ch_cur == 1)
        {
            stopMKO();
            Sleep(100);
            emit MKO_CTM(2, 1);
            Sleep(1200);
            startMKO1();
        }

        if(ch_cur == 2)
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
        if (tmkselect(1)!= 0)
        {
            err3 += "Ошибка! tmk0!\n";
        }

        bcreset();
        Sleep(100);
        SubAddrOU = 13;
        msgCmd = (AddrOU << 11) + RT_TRANSMIT + (SubAddrOU << 5) + (21 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bus = CX_BUS_0;
        bcstartx(0, DATA_RT_BC | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff2, 22);
        err3 += OCcontrol(buff2[0]);
        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff2[i]) + " ";
        }

        if (tmkselect(0)!= 0)
        {
            err3 += "Ошибка! tmk1!\n";
        }

        bcreset();
        Sleep(100);
        err3 += " Резервный полукомплект прием массива: \n";
        AddrOU = adr2;
        SubAddrOU = 13;
        msgCmd = (AddrOU << 11) + RT_TRANSMIT + (SubAddrOU << 5) + (21 & NWORDS_MASK);
        bcdefbase(0);
        bcputw(0, msgCmd);
        bus = CX_BUS_0;
        bcstartx(0, DATA_RT_BC | bus | CX_STOP | CX_NOSIG);
        Sleep(100);
        bcgetblk(0, buff2, 22);
        err3 += OCcontrol(buff2[0]);
        for (int i = 1; i < 22; i++)
        {
            data += QString::number(buff2[i]) + " ";
        }
    }

    emit data_MKO(data);
    emit start_MKO(err3);
}
