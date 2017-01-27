#include "Headers/system/modules/module_otd.h"
#include <QDebug>
#include <windows.h>
#include <QTimer>
#include <QTime>
#include "qapplication.h"
#include <QtSerialPort>

#include "Headers/module_commands.h"
#include "Headers/logger/Logger.h"


namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const int SERIAL_NUMER_BYTES_COUNT = 8;
    static const int MAX_PT100_COUNT = 2;
}

ModuleOTD::ModuleOTD(QObject* parent):
    ModuleOKB(parent),
    m_sensorsCntAxis1(0),
    m_sensorsCntAxis2(0)

{
    //m_timer = new QTimer(this);
    //m_timer->setSingleShot(true);
    //connect(m_timer, SIGNAL(timeout()), this, SLOT(OTD_timer()));
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

bool ModuleOTD::postInitOKBModule()
{
    int TODO;
    int i = 0;

    //readDS1820Data(PSY);
    //readDS1820Data(NU);

    // 1. Read sensors count on both lines
    // 2. Read sensors addresses on both lines
    // 3. Start measurement on both lines
    // 4. Get temperature on all sensors on both lines
    // 5. Get PT-100 temperature value
    // 6. Set received data to system state?
    // 7. Launch update timer

    return true;
}

void ModuleOTD::measureDS1820(LineID line)
{
    // start measurement
    ModuleCommands::CommandID command = (line == PSY) ? ModuleCommands::START_MEASUREMENT_LINE_1 : ModuleCommands::START_MEASUREMENT_LINE_2;
    if (!sendCommand(command, 0, 0))
    {
        int TODO;
        //emit err_OTD("Ошибка при перезагрузке!");
        return;
    }

    int count = 0;
    if (line == PSY)
    {
        count = m_sensorsCntAxis1;
    }
    else
    {
        count = m_sensorsCntAxis2;
    }

    // get temperature data
    for(int i = 1; i <= count; i++)
    {
        ModuleCommands::CommandID getCommand = (line == PSY) ? ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1 : ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2;
        QByteArray response;
        if (!sendCommand(getCommand, i, 0, &response))
        {
            continue;
            int TODO;
            //emit err_OTD("Ошибка при перезагрузке!");
        }

        uint8_t uu1, uu2, z;
        uu1 = response[2];
        uu2 = response[3];
        double uu = (uu1 << 8) | uu2;
        uint8_t x = response[2];
        z = x << 4;
        z = z >> 7;
        if (z == 0) //TODO определить что такое 0
        {
            uu = uu / 16;
        }

        if (z == 1) // определить что такое 1
        {
            uu = (uu - 4096) / 16;
        }

        //uu - темепература, вероятно, сохранить
    }
}

void ModuleOTD::measurePT100()
{
    double values[MAX_PT100_COUNT];

    for (int i = 0; i < MAX_PT100_COUNT; ++i)
    {
        QByteArray response;
        if (!sendCommand(ModuleCommands::GET_TEMPERATURE_PT100, i + 1, 0, &response))
        {
            int TODO;
            continue;
        }

        uint8_t uu1, uu2;
        uu1 = response[2];
        uu2 = response[3];
        double uu = (uu1 << 8) | uu2;
        uu = uu / 32 - 256;

        values[i] = uu * 100;
    }

    //emit start_OTDPT(values[0], values[1]); // TODO hardcode
    /*
     *
     * int TODO; // ошибки ПТ-100 датчиков (см ниже)

    x = x / 100;
    y = y / 100;
    //ui->OTDerror->setStyleSheet("font: 25 12pt GOST type A;" "color: red;");
    //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
    //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    //ui->OTDPT1->setText(QString::number(x));
    //ui->OTDPT2->setText(QString::number(y));
*/
}

void ModuleOTD::readDS1820Data(LineID line)
{
    QByteArray response;

    // get sensors count
    ModuleCommands::CommandID commandGetCount = (line == PSY) ? ModuleCommands::GET_DS1820_COUNT_LINE_1 : ModuleCommands::GET_DS1820_COUNT_LINE_2;
    int count = 0;

    if (!sendCommand(commandGetCount, 0, 0, &response))
    {
        int TODO;
        return;
    }

    if (line == PSY)
    {
        m_sensorsCntAxis1 = response[2];
        count = m_sensorsCntAxis1;
        LOG_INFO("DS1820 sensors count on 1 (PSY) line is %i", count);
    }
    else
    {
        m_sensorsCntAxis2 = response[2];
        count = m_sensorsCntAxis2;
        LOG_INFO("DS1820 sensors count on 2 (NU) line is %i", count);
    }

    // get sensors adresses
    ModuleCommands::CommandID commandGetAddr = (line == PSY) ? ModuleCommands::GET_DS1820_ADDR_LINE_1 : ModuleCommands::GET_DS1820_ADDR_LINE_2;

    for(int j = 1; j <= count; j++)
    {
        for(int k = 0; k < SERIAL_NUMER_BYTES_COUNT; k++)
        {
            QByteArray response1;
            if (!sendCommand(commandGetAddr, j, k, &response1))
            {
                int TODO;
                continue;
            }

            uint8_t addr = response1[2];
            LOG_INFO("DS1820 sensor %i address is %i", j, addr);
        }
    }

    int TODO;
    //if(!readData0.isEmpty() && readData0.at(3) == 2)
    //{
    //    data += "Ошибка при считывании датчиков 1-й оси\n";
    //}
}

void ModuleOTD::processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    int TODO; // just execute and fill response, do not send any signals
}
