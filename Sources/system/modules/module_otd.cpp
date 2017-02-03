#include "Headers/system/modules/module_otd.h"
#include "Headers/module_commands.h"
#include "Headers/logger/Logger.h"
#include "Headers/system/system_state.h"

//#include <QTimer>

namespace
{
    static const uint8_t OTD_DEFAULT_ADDR = 0x44;
    static const int SERIAL_NUMBER_BYTES_COUNT = 8;
    static const int MAX_PT100_COUNT = 2;
}

ModuleOTD::ModuleOTD(QObject* parent):
    ModuleOKB(parent),
    mSensorsCntPsy(0),
    mSensorsCntNu(0)
{
    //mTimer = new QTimer(this);
    //mTimer->setSingleShot(true);
    //connect(mTimer, SIGNAL(timeout()), this, SLOT(OTD_timer()));
}

ModuleOTD::~ModuleOTD()
{

}

int ModuleOTD::ptCount() const
{
    return MAX_PT100_COUNT;
}

int ModuleOTD::dsCount(LineID line) const
{
    if (line == PSY)
    {
        return mSensorsCntPsy;
    }

    return mSensorsCntNu;
}

void ModuleOTD::resetLine(LineID line)
{
    ModuleCommands::CommandID command = (line == PSY) ? ModuleCommands::RESET_LINE_1 : ModuleCommands::RESET_LINE_2;
    if (!sendCommand(command, 0, 0))
    {
        int TODO;
    }
}

bool ModuleOTD::postInitOKBModule()
{
    resetError();

    QByteArray response;
    sendCommand(ModuleCommands::RESET_LINE_2, 0, 0, &response);
    response.clear();
    sendCommand(ModuleCommands::RESET_LINE_1, 0, 0, &response);

    // 1. Read sensors count on both lines
    // 2. Read sensors addresses on both lines
    // 3. Start measurement on both lines
    // 4. Get temperature on all sensors on both lines
    // 5. Get PT-100 temperature value

    readDS1820Data(PSY);
    readDS1820Data(NU);

    return true;
}

void ModuleOTD::measureDS1820(LineID line, QList<qreal>& values)
{
    // start measurement
    LOG_INFO("Start temperature measurement at line %i", line);
    ModuleCommands::CommandID command = (line == PSY) ? ModuleCommands::START_MEASUREMENT_LINE_1 : ModuleCommands::START_MEASUREMENT_LINE_2;
    if (!sendCommand(command, 0, 0))
    {
        return;
    }

    LOG_INFO("Retreive temperature data from line %i", line);
    int count = 0;
    if (line == PSY)
    {
        count = mSensorsCntPsy;
    }
    else
    {
        count = mSensorsCntNu;
    }

    // get temperature data
    for(int i = 0; i < count; ++i)
    {
        ModuleCommands::CommandID getCommand = (line == PSY) ? ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1 : ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2;
        QByteArray response;
        if (!sendCommand(getCommand, i + 1, 0, &response))
        {
            continue;
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

        if (z == 1) //TODO определить что такое 1
        {
            uu = (uu - 4096) / 16;
        }

        LOG_INFO("Sensor %i temperature is %f", i + 1, uu);
        values.push_back(uu);
    }
}

void ModuleOTD::measurePT100(QList<qreal>& values)
{
    LOG_INFO("Start temperature measurement with PT100 sensors");
    for (int i = 0; i < MAX_PT100_COUNT; ++i)
    {
        QByteArray response;
        if (!sendCommand(ModuleCommands::GET_TEMPERATURE_PT100, i + 1, 0, &response))
        {
            continue;
        }

        uint8_t uu1, uu2;
        uu1 = response[2];
        uu2 = response[3];
        double uu = (uu1 << 8) | uu2;
        uu = uu / 32 - 256;

        values.push_back(uu * 100);
        LOG_INFO("PT100 sensor %i temperature is %f", i + 1, uu);

        //x = x / 100;
        //y = y / 100;
        //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
        //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
        //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
        //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
    }
}

void ModuleOTD::readDS1820Data(LineID line)
{
    LOG_INFO("Read line %i sensors data", line);
    QByteArray response;

    // get sensors count
    ModuleCommands::CommandID commandGetCount = (line == PSY) ? ModuleCommands::GET_DS1820_COUNT_LINE_1 : ModuleCommands::GET_DS1820_COUNT_LINE_2;

    if (!sendCommand(commandGetCount, 0, 0, &response))
    {
        return;
    }

    int count = response[2];
    if (line == PSY)
    {
        mSensorsCntPsy = count;
        LOG_INFO("DS1820 sensors count at 1 (PSY) line is %i", count);
    }
    else
    {
        mSensorsCntNu = count;
        LOG_INFO("DS1820 sensors count at 2 (NU) line is %i", count);
    }

    // get sensors adresses TODO (пока пофиг)
    /*
    ModuleCommands::CommandID commandGetAddr = (line == PSY) ? ModuleCommands::GET_DS1820_ADDR_LINE_1 : ModuleCommands::GET_DS1820_ADDR_LINE_2;

    for(int j = 0; j < count; ++j)
    {
        for(int k = 0; k < SERIAL_NUMBER_BYTES_COUNT; ++k)
        {
            QByteArray response1;
            if (!sendCommand(commandGetAddr, j + 1, k, &response1))
            {
                continue;
            }

            uint8_t addr = response1[2];
            //LOG_INFO("DS1820 sensor %i address is %i", j + 1, addr);
        }
    }*/
}

void ModuleOTD::processCustomCommand(const QMap<uint32_t, QVariant>& request, QMap<uint32_t, QVariant>& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(request.value(SystemState::COMMAND_ID).toUInt());
    QList<qreal> temperature;

    switch (command)
    {
    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            measurePT100(temperature);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            measureDS1820(PSY, temperature);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        {
            measureDS1820(NU, temperature);
        }
        break;

    default:
        LOG_ERROR("Unexpected command %i received by OTD module", command);
        break;
    }

    if (!temperature.empty())
    {
        // fill response
        int paramsCount = request.value(SystemState::OUTPUT_PARAMS_COUNT, 0).toInt();
        int valuesCount = temperature.size();

        if (paramsCount != valuesCount)
        {
            LOG_ERROR("Request output params count (%i) and values count (%i) mismatch", paramsCount, valuesCount);
            return;
        }

        for (int i = 0; i < paramsCount; ++i)
        {
            uint32_t paramType = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2).toUInt();
            QString varName    = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2 + 1).toString();

            response[SystemState::OUTPUT_PARAM_BASE + i * 2] = QVariant(varName);
            response[SystemState::OUTPUT_PARAM_BASE + i * 2 + 1] = QVariant(temperature[i]);
        }

        response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount * 2);
    }
}
