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

    double getDS1820Temp(const QByteArray& response)
    {
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

        return uu;
    }

    double getPT100Temp(const QByteArray& response)
    {
        uint8_t uu1, uu2;
        uu1 = response[2];
        uu2 = response[3];
        double uu = (uu1 << 8) | uu2;
        uu = uu / 32 - 256;

        int TODO; // parse error
        //x = x / 100;
        //y = y / 100;
        //if(x == -256) ui->OTDerror->setText("Ошибка измерения датчика");
        //if(y == -256) ui->OTDerror->setText("Ошибка измерения датчика");
        //if(x > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");
        //if(y > 1790) ui->OTDerror->setText("Ошибка обращения к модулю датчика");

        return uu;
    }
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

void ModuleOTD::initializeCustomOKBModule()
{
    // 1. Reset sensors on both lines
    // 2. Read sensors count on both lines
    // 3. Read sensors addresses on both lines (OPTIONAL)

    // reset sensors on both lines (it doesn't work without that :))
    addCommandToQueue(ModuleCommands::RESET_LINE_1, 0, 0);
    addCommandToQueue(ModuleCommands::RESET_LINE_2, 0, 0);

    // read sensors count on both lines (TODO do not change call order)
    addCommandToQueue(ModuleCommands::GET_DS1820_COUNT_LINE_1, 0, 0);
    addCommandToQueue(ModuleCommands::GET_DS1820_COUNT_LINE_2, 0, 0);

    // get sensors adresses TODO (what for this functionality is used?)
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
    mTemperatureData.clear();
    ModuleCommands::CommandID command = ModuleCommands::CommandID(request.value(SystemState::COMMAND_ID).toUInt());

    switch (command)
    {
    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            LOG_INFO("Start temperature measurement with PT100 sensors");
            for (int i = 0; i < MAX_PT100_COUNT; ++i)
            {
                addCommandToQueue(ModuleCommands::GET_TEMPERATURE_PT100, i + 1, 0);
            }
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            LOG_INFO("Start temperature measurement at line 1");
            addCommandToQueue(ModuleCommands::START_MEASUREMENT_LINE_1, 0, 0);
            for(int i = 0; i < mSensorsCntPsy; ++i)
            {
                addCommandToQueue(command, i + 1, 0);
            }
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        {
            LOG_INFO("Start temperature measurement at line 2");
            addCommandToQueue(ModuleCommands::START_MEASUREMENT_LINE_2, 0, 0);
            for(int i = 0; i < mSensorsCntNu; ++i)
            {
                addCommandToQueue(command, i + 1, 0);
            }
        }
        break;

    default:
        LOG_ERROR("Unexpected command %i received by OTD module", command);
        return;
        break;
    }

    mRequestQueue.back().response = response;

    // set variables
    int paramsCount = request.value(SystemState::OUTPUT_PARAMS_COUNT).toInt();
    for (int i = 0; i < paramsCount; ++i)
    {
        QString varName = request.value(SystemState::OUTPUT_PARAM_BASE + i * 2 + 1).toString();
        mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + i * 2] = QVariant(varName);
    }

    mRequestQueue.back().response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount);
}

void ModuleOTD::processCustomResponse(const QByteArray& response)
{
    ModuleCommands::CommandID command = ModuleCommands::CommandID(mRequestQueue.front().operation);

    switch (command)
    {
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        {
            mSensorsCntPsy = response[2];
            LOG_INFO("DS1820 sensors count at line 1 is %i", mSensorsCntPsy);
        }
        break;

    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
        {
            mSensorsCntNu = response[2];
            LOG_INFO("DS1820 sensors count at line 2 is %i", mSensorsCntNu);

            if (!mCustomInitializationFinished)
            {
                mCustomInitializationFinished = true;
                emit initializationFinished(QString(""));
            }
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            double uu = getDS1820Temp(response);
            LOG_INFO("Sensor #%i at line 1 temperature is %f", mTemperatureData.size() + 1, uu);
            mTemperatureData.push_back(uu);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        {
            double uu = getDS1820Temp(response);
            LOG_INFO("Sensor #%i at line 2 temperature is %f", mTemperatureData.size() + 1, uu);
            mTemperatureData.push_back(uu);
        }
        break;

    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            double uu = getPT100Temp(response);
            LOG_INFO("PT100 sensor %i temperature is %f", mTemperatureData.size() + 1, uu);
            mTemperatureData.push_back(uu);
        }
        break;


    case ModuleCommands::START_MEASUREMENT_LINE_1:
        {
            int TODO; // check error
        }
        break;

    case ModuleCommands::START_MEASUREMENT_LINE_2:
        {
            int TODO; // check error
        }
        break;

    case ModuleCommands::RESET_LINE_1:
        {
            int TODO; // check error
        }
        break;

    case ModuleCommands::RESET_LINE_2:
        {
            int TODO; // check error
        }
        break;

    default:
        LOG_WARNING(QString("Unexpected command id=0x%1 response received by OTD module: %2").arg(QString::number(command, 16)).arg(QString(response.toHex().toStdString().c_str())));
        break;
    }

    if (!mRequestQueue.back().response.empty())
    {
        // fill response
        int paramsCount = mRequestQueue.back().response.value(SystemState::OUTPUT_PARAMS_COUNT, 0).toInt();
        int valuesCount = mTemperatureData.size();

        if (paramsCount != valuesCount)
        {
            LOG_ERROR("Request output params count (%i) and values count (%i) mismatch", paramsCount, valuesCount);
            return;
        }

        for (int i = 0; i < paramsCount; ++i)
        {
            mRequestQueue.back().response[SystemState::OUTPUT_PARAM_BASE + i * 2 + 1] = QVariant(mTemperatureData[i]);
        }

        mRequestQueue.back().response[SystemState::OUTPUT_PARAMS_COUNT] = QVariant(paramsCount * 2);
    }
}

void ModuleOTD::onApplicationFinish()
{
    int TODO;
}
