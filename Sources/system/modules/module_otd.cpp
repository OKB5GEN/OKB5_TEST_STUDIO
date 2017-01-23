#include "Headers/system/modules/module_otd.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include "qapplication.h"
#include <QtSerialPort>
//#include "synchapi.h"
#include "Headers/module_commands.h"


namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const int SERIAL_NUMER_BYTES_COUNT = 8;
    static const int MAX_PT100_COUNT = 2;
}

ModuleOTD::ModuleOTD(QObject* parent):
    ModuleOKB(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(OTD_timer()));
}

ModuleOTD::~ModuleOTD()
{

}

void ModuleOTD::resetLine(LineID line)
{
    ModuleCommands::CommandID command = (line == PSY) ? ModuleCommands::RESET_LINE_1 : ModuleCommands::RESET_LINE_2;
    if (!sendCommand(command, 0, 0))
    {
        int TODO;
        //emit err_OTD("Ошибка при перезагрузке!");
    }
}

void ModuleOTD::postInit1()
{
    int TODO;

    // 1. Read sensors count on both lines
    // 2. Read sensors addresses on both lines
    // 3. Start measurement on both lines
    // 4. Get temperature on all sensors on both lines
    // 5. Get PT-100 temperature value
    // 6. Set received data to system state?
    // 7. Launch update timer
}

QByteArray ModuleOTD::send1(QByteArray data, double readTimeout, double delayBeforeRecv /*= 0*/)
{
    QByteArray readData;
    if (mPort && mPort->isOpen())
    {
        mPort->QIODevice::write(data);
        mPort->waitForBytesWritten(-1);

        if (delayBeforeRecv > 0)
        {
            Sleep(delayBeforeRecv);
        }

        readData = mPort->readAll();
        while (mPort->waitForReadyRead(readTimeout))
        {
            readData.append(mPort->readAll());
        }
    }

    return readData;
}

void ModuleOTD::COMConnectorOTD()
{
    m_isActive = true;
    OTD_id();
    OTDtemper();
}

void ModuleOTD::OTD_id()
{
    QByteArray bw(4, 0);
    bw[0] = 0xff;
    bw[1] = ModuleCommands::GET_MODULE_ADDRESS;
    bw[2] = 0x00;
    bw[3] = ModuleCommands::CURRENT;
    QByteArray readData1 = send1(bw, 100);

    bw[0] = 0xff;
    bw[1] = ModuleCommands::GET_MODULE_ADDRESS;
    bw[2] = 0x00;
    bw[3] = ModuleCommands::DEFAULT;

    QByteArray readData0 = send1(bw, 500);

    if(readData1[2] != readData0[2] || readData1[3] != readData0[3])
    {
        emit OTD_id1();
    }
}

void ModuleOTD::OTD_req()
{
    if(m_isActive)
    {
        QString res;
        QByteArray bw(4, 0);
        bw[0] = OTD_DEFAULT_ADDR;
        bw[1] = ModuleCommands::GET_STATUS_WORD;
        bw[2] = 0x00;
        bw[3] = 0x00;
        QByteArray readData1 = send1(bw, 100);
        uint8_t x = readData1[2];
        uint8_t z1, z2, z3;
        res = "";
        z1 = x >> 7;
        z2 = x << 1;
        z2 = z2 >> 7;
        z3 = x << 2;
        z3 = z3 >> 7;
        if (z1 == 0)
        {
            res += " ОТД не готов к работе! \n";
        }

        if (z2 == 1)
        {
            res += " Ошибки у модуля ОТД! \n";
        }

        if (z3 == 1)
        {
            res += " Модуль ОТД после перезагрузки! \n";
        }

        emit OTD_reqr (res);
    }
}

void ModuleOTD::OTD_fw()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::GET_SOWFTWARE_VER;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 100);
    emit OTD_vfw(readData1[2] * 10 + readData1[3]);
}

void ModuleOTD::res_OTD()
{
    m_isActive = false;
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::SOFT_RESET;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 100);

    if (mPort)
    {
        mPort->close();
    }

    for(int i = 0; i < 400; i++) // TODO Remove this shit
    {
        Sleep(10);
        QApplication::processEvents();
    }

    COMConnectorOTD();
    emit OTD_res(readData1[3]);
}

void ModuleOTD::err_res_OTD()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::RESET_ERROR;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 100);
    emit OTD_err_res(readData1[3]);
}

void ModuleOTD::OTDres1()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::RESET_LINE_1;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 500);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при перезагрузке!");
    }
}

void ModuleOTD::OTDres2()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::RESET_LINE_2;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 500);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при перезагрузке!");
    }
    else
    {
        emit err_OTD("");
    }
}

void ModuleOTD::OTDmeas1()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::START_MEASUREMENT_LINE_1;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 500, 2000);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при запуске измерений 1-й линии!");
    }

    OTDtm1();
}

void ModuleOTD::OTDmeas2()
{
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::START_MEASUREMENT_LINE_2;
    bw[2] = 0x00;
    bw[3] = 0x00;
    QByteArray readData1 = send1(bw, 500, 2000);
    if(readData1.at(3) == 2)
    {
        emit err_OTD("Ошибка при запуске измерений 2-й линии!");
    }

    OTDtm2();
}

void ModuleOTD::OTDtemper()
{
    QString data;
    data += "Кол-во датчиков DS1820 по оси 1: ";
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::GET_DS1820_COUNT_LINE_1;
    bw[2] = 0x00;
    bw[3] = 0x00;

    QByteArray readData0 = send1(bw, 500);
    m_sensorsCntAxis1 = readData0[2];
    data += QString::number(m_sensorsCntAxis1);
    data += "\n";
    data += "Адреса датчиков по оси 1: \n";

    for(int j = 1; j <= m_sensorsCntAxis1; j++)
    {
        data += QString::number(j);
        data += " : ";
        for(int k = 0; k < SERIAL_NUMER_BYTES_COUNT; k++)
        {
            bw[0] = OTD_DEFAULT_ADDR;
            bw[1] = ModuleCommands::GET_DS1820_ADDR_LINE_1;
            bw[2] = j;
            bw[3] = k;
            QByteArray readData3 = send1(bw, 100);
            data += QString::number(readData3[2], 16);
        }

        data += "\n";
    }

    data += "\n";
    if(!readData0.isEmpty() && readData0.at(3) == 2)
    {
        data += "Ошибка при считывании датчиков 1-й оси\n";
    }

    data += "Кол-во датчиков DS1820 по оси 2: ";
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::GET_DS1820_COUNT_LINE_2;
    bw[2] = 0x00;
    bw[3] = 0x00;

    QByteArray readData3 = send1(bw, 500);
    m_sensorsCntAxis2 = readData3[2];
    data += QString::number(m_sensorsCntAxis2);
    data += "\n";
    data += "Адреса датчиков по оси 2: \n";
    for(int j = 1; j <= m_sensorsCntAxis2; j++)
    {
        data += QString::number(j);
        data += " : ";
        for(int k = 0; k < SERIAL_NUMER_BYTES_COUNT; k++)
        {
            bw[0] = OTD_DEFAULT_ADDR;
            bw[1] = ModuleCommands::GET_DS1820_ADDR_LINE_2;
            bw[2] = j;
            bw[3] = k;
            QByteArray readData4 = send1(bw, 100);
            data += QString::number(readData4[2], 16);
        }

        data += "\n";
    }

    if (!readData3.isEmpty() && readData3.at(3) == 2)
    {
        data += "Ошибка при считывании датчиков 2-й оси\n";
    }

    emit temp_OTD(data);
}

void ModuleOTD::OTDtm1()
{
    QString temp;
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1;
    bw[3] = 0x00;

    for(int i = 1; i <= m_sensorsCntAxis1; i++)
    {
        temp += " Температура датчиков 1-й линии \n ";
        temp += QString::number(i);
        temp += " : ";
        bw[2] = i;
        QByteArray readData1 = send1(bw, 500);
        uint8_t uu1, uu2, z;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double uu = (uu1 << 8) | uu2;
        uint8_t x = readData1[2];
        z = x << 4;
        z = z >> 7;
        if (z == 0)
        {
            uu = uu / 16;
        }

        if (z == 1)
        {
            uu = (uu - 4096) / 16;
        }

        temp += QString::number(uu);
        temp += "\n";
    }

    emit tm_OTD1(temp);
}

void ModuleOTD::OTDtm2()
{
    QString temp;
    QByteArray bw(4, 0);
    bw[0] = OTD_DEFAULT_ADDR;
    bw[1] = ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2;
    bw[3] = 0x00;

    for(int i = 1; i <= m_sensorsCntAxis2; i++)
    {
        temp += " Температура датчиков 2-й линии \n ";
        temp += QString::number(i);
        temp += " : ";
        bw[2] = i;
        QByteArray readData1 = send1(bw, 500);
        uint8_t uu1, uu2, z;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double uu = (uu1 << 8) | uu2;
        uint8_t x = readData1[2];
        z = x << 4;
        z = z >> 7;
        if (z == 0)
        {
            uu = uu / 16;
        }

        if (z == 1)
        {
            uu = (uu - 4096) / 16;
        }

        temp += QString::number(uu);
        temp += "\n";
    }

    emit tm_OTD2(temp);
}

void ModuleOTD::OTDPT()
{
    double values[MAX_PT100_COUNT];

    for (int i = 0; i < MAX_PT100_COUNT; ++i)
    {
        QByteArray bw(4, 0);
        bw[0] = OTD_DEFAULT_ADDR;
        bw[1] = ModuleCommands::GET_TEMPERATURE_PT100;
        bw[2] = i + 1;
        bw[3] = 0x00;
        QByteArray readData1 = send1(bw, 500);
        uint8_t uu1, uu2;
        uu1 = readData1[2];
        uu2 = readData1[3];
        double uu = (uu1 << 8) | uu2;
        uu = uu / 32 - 256;

        values[i] = uu * 100;
    }

    emit start_OTDPT(values[0], values[1]); // TODO hardcode
}

void ModuleOTD::OTD_avt(int x, int y)
{
    if(x == 1)
    {
        m_timer->start(y);
    }
    else
    {
        y = 0;
    }
}

void ModuleOTD::OTD_timer()
{
    OTDPT();
    OTDmeas1();
    OTDmeas2();
}

void ModuleOTD::doWork()
{
    emit start_OTD();
    int TODO; // ошибки ПТ-100 датчиков (см ниже)
}

/*
void SystemState::OTDPTdata(double x,double y)
{
    x = x / 100;
    y = y / 100;
    //ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //ui->OTDPT1->setText(QString::number(x));
    //ui->OTDPT2->setText(QString::number(y));
}*/
