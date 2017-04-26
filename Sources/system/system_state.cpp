#include "Headers/system/system_state.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>
#include <QtSerialPort>
#include <windows.h>
#include "qapplication.h"

namespace
{
    bool loadSystemConfig(QMap<ModuleCommands::ModuleID, COMPortModule::Identifier>& modules, bool& emulatorEnabled)
    {
        modules.clear();

        QXmlStreamReader xml;

        QString fileName = QDir::currentPath() + "/system_config.xml";
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            LOG_FATAL("No system_config.xml found");
            //QMessageBox::warning(this, tr("OKB5 Test Studio"), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return false;
        }

        xml.setDevice(&file);
        QMetaEnum metaEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();

        emulatorEnabled = false;

        if (xml.readNextStartElement())
        {
            if (xml.name() == "system_config" && xml.attributes().value("version") == "1.0")
            {
                while (!xml.atEnd() && !xml.hasError())
                {
                    QXmlStreamReader::TokenType token = xml.readNext();

                    if (token == QXmlStreamReader::StartElement)
                    {
                        QString name = xml.name().toString();

                        if (name == "modules")
                        {
                            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "modules"))
                            {
                                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "module")
                                {
                                    QXmlStreamAttributes attributes = xml.attributes();

                                    COMPortModule::Identifier id;
                                    ModuleCommands::ModuleID moduleID = ModuleCommands::MODULES_COUNT;

                                    if (attributes.hasAttribute("type"))
                                    {
                                        QString str = attributes.value("type").toString();
                                        moduleID = ModuleCommands::ModuleID(metaEnum.keyToValue(qPrintable(str)));
                                    }

                                    if (attributes.hasAttribute("description"))
                                    {
                                        id.description = attributes.value("description").toString();
                                    }

                                    if (attributes.hasAttribute("productId"))
                                    {
                                        id.productId = attributes.value("productId").toUShort();
                                    }

                                    if (attributes.hasAttribute("serialNumber"))
                                    {
                                        id.serialNumber = attributes.value("serialNumber").toString();
                                    }

                                    if (attributes.hasAttribute("vendorId"))
                                    {
                                        id.vendorId = attributes.value("vendorId").toUShort();
                                    }

                                    if (moduleID != ModuleCommands::MODULES_COUNT)
                                    {
                                        modules[moduleID] = id;
                                    }
                                }

                                xml.readNext();
                            }
                        }
                        else if (name == "emulator")
                        {
                            QXmlStreamAttributes attributes = xml.attributes();

                            if (attributes.hasAttribute("enabled"))
                            {
                                emulatorEnabled = (attributes.value("enabled").toInt() != 0);
                            }
                        }
                    }
                }
            }
            else
            {
                xml.raiseError(QObject::tr("The file is not an system_config version 1.0 file."));
            }
        }

        if (!xml.errorString().isEmpty())
        {
            LOG_ERROR(QObject::tr("%1\nLine %2, column %3")
                      .arg(xml.errorString())
                      .arg(xml.lineNumber())
                      .arg(xml.columnNumber()));
        }

        return !xml.error();
    }
}

///////////////////////////////////////////////////////////////
SystemState::SystemState(QObject* parent):
    QObject(parent),
    mCurCommand(Q_NULLPTR),
    mMKO(Q_NULLPTR),
    mOTD(Q_NULLPTR),
    mSTM(Q_NULLPTR),
    mTech(Q_NULLPTR),
    mPowerBUP(Q_NULLPTR),
    mPowerPNA(Q_NULLPTR)
{
    mParamNames[VOLTAGE] = tr("Voltage, V");
    mParamNames[CURRENT] = tr("Current, A");
    mParamNames[TEMPERATURE] = tr("Temperature, °C");
    mParamNames[MODE] = tr("Mode");
    mParamNames[STEPS] = tr("Steps");
    mParamNames[VELOCITY] = tr("Velocity");
    mParamNames[MODE_PSY] = tr("Mode (Psy)");
    mParamNames[STEPS_PSY] = tr("Steps (Psy)");
    mParamNames[VELOCITY_PSY] = tr("Velocity (Psy)");
    mParamNames[CURRENT_PSY] = tr("Max current (Psy)");
    mParamNames[ANGLE_PSY] = tr("Angle (Psy)");
    mParamNames[MODE_NU] = tr("Mode (Nu)");
    mParamNames[STEPS_NU] = tr("Steps (Nu)");
    mParamNames[VELOCITY_NU] = tr("Velocity (Nu)");
    mParamNames[CURRENT_NU] = tr("Max current (Nu)");
    mParamNames[ANGLE_NU] = tr("Angle (Nu)");
    mParamNames[SENSOR_FLAG] = tr("Temp.Sensor Flag");

    // implicit params
    mParamNames[SUBADDRESS] = tr("Subaddress");
    mParamNames[POWER_STATE] = tr("Power State");
    mParamNames[CHANNEL_ID] = tr("Channel");

    mDefaultVariables[VOLTAGE] = "U";
    mDefaultVariables[CURRENT] = "I";
    mDefaultVariables[TEMPERATURE] = "T";
    mDefaultVariables[MODE] = "M";
    mDefaultVariables[STEPS] = "St";
    mDefaultVariables[VELOCITY] = "V";
    mDefaultVariables[MODE_PSY] = "PsyM";
    mDefaultVariables[STEPS_PSY] = "PsyS";
    mDefaultVariables[VELOCITY_PSY] = "PsyV";
    mDefaultVariables[CURRENT_PSY] = "PsyI";
    mDefaultVariables[ANGLE_PSY] = "PsyA";
    mDefaultVariables[MODE_NU] = "NuM";
    mDefaultVariables[STEPS_NU] = "NuS";
    mDefaultVariables[VELOCITY_NU] = "NuV";
    mDefaultVariables[CURRENT_NU] = "NuI";
    mDefaultVariables[ANGLE_NU] = "NuA";
    mDefaultVariables[SENSOR_FLAG] = "TSF";

    mDefaultDescriptions[VOLTAGE] = tr("Voltage, V");
    mDefaultDescriptions[CURRENT] = tr("Current, A");
    mDefaultDescriptions[TEMPERATURE] = tr("Temperature, °C");
    mDefaultDescriptions[MODE] = tr("Mode");
    mDefaultDescriptions[STEPS] = tr("Steps");
    mDefaultDescriptions[VELOCITY] = tr("Velocity");
    mDefaultDescriptions[MODE_PSY] = tr("Mode (Psy)");
    mDefaultDescriptions[STEPS_PSY] = tr("Steps (Psy)");
    mDefaultDescriptions[VELOCITY_PSY] = tr("Velocity (Psy)");
    mDefaultDescriptions[CURRENT_PSY] = tr("Max current (Psy)");
    mDefaultDescriptions[ANGLE_PSY] = tr("Angle (Psy)");
    mDefaultDescriptions[MODE_NU] = tr("Mode (Nu)");
    mDefaultDescriptions[STEPS_NU] = tr("Steps (Nu)");
    mDefaultDescriptions[VELOCITY_NU] = tr("Velocity (Nu)");
    mDefaultDescriptions[CURRENT_NU] = tr("Max current (Nu)");
    mDefaultDescriptions[ANGLE_NU] = tr("Angle (Nu)");
    mDefaultDescriptions[SENSOR_FLAG] = tr("Temperature sensor presense flag");
}

SystemState::~SystemState()
{
}

void SystemState::onApplicationStart()
{
    // Load modules configuration file
    bool emulatorEnabled = false;
    QMap<ModuleCommands::ModuleID, COMPortModule::Identifier> modules;
    loadSystemConfig(modules, emulatorEnabled);

    QString mode = emulatorEnabled ? tr("EMULATOR") : tr("REAL DEVICE");
    LOG_INFO(QString("System mode: %1").arg(mode));

    // Create modules objects
    mMKO = new ModuleMKO(this);
    mMKO->setEmulator(emulatorEnabled);
    mMKO->setModuleID(ModuleCommands::MKO);
    connect(this, SIGNAL(sendToMKO(const Transaction&)), mMKO, SLOT(processCommand(const Transaction&)));
    connect(mMKO, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mMKO, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mOTD = new ModuleOTD(this);
    mOTD->setEmulator(emulatorEnabled);
    mOTD->setId(modules.value(ModuleCommands::OTD));
    mOTD->setModuleID(ModuleCommands::OTD);
    connect(this, SIGNAL(sendToOTD(const Transaction&)), mOTD, SLOT(processCommand(const Transaction&)));
    connect(mOTD, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mOTD, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mSTM = new ModuleSTM(this);
    mSTM->setEmulator(emulatorEnabled);
    mSTM->setId(modules.value(ModuleCommands::STM));
    mSTM->setModuleID(ModuleCommands::STM);
    connect(this, SIGNAL(sendToSTM(const Transaction&)), mSTM, SLOT(processCommand(const Transaction&)));
    connect(mSTM, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mSTM, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mTech = new ModuleTech(this);
    mTech->setEmulator(emulatorEnabled);
    mTech->setId(modules.value(ModuleCommands::TECH));
    mTech->setModuleID(ModuleCommands::TECH);
    connect(this, SIGNAL(sendToTech(const Transaction&)), mTech, SLOT(processCommand(const Transaction&)));
    connect(mTech, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mTech, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mPowerBUP = new ModulePower(this);
    mPowerBUP->setEmulator(emulatorEnabled);
    mPowerBUP->setId(modules.value(ModuleCommands::POWER_UNIT_BUP));
    mPowerBUP->setModuleID(ModuleCommands::POWER_UNIT_BUP);
    connect(this, SIGNAL(sendToPowerUnitBUP(const Transaction&)), mPowerBUP, SLOT(processCommand(const Transaction&)));
    connect(mPowerBUP, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mPowerBUP, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

    mPowerPNA = new ModulePower(this);
    mPowerPNA->setEmulator(emulatorEnabled);
    mPowerPNA->setId(modules.value(ModuleCommands::POWER_UNIT_PNA));
    mPowerPNA->setModuleID(ModuleCommands::POWER_UNIT_PNA);
    connect(this, SIGNAL(sendToPowerUnitPNA(const Transaction&)), mPowerPNA, SLOT(processCommand(const Transaction&)));
    connect(mPowerPNA, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));
//    connect(mPowerPNA, SIGNAL(stateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)), this, SLOT(onModuleStateChanged(ModuleCommands::ModuleID, AbstractModule::ModuleState, AbstractModule::ModuleState)));

//    mModules[ModuleCommands::MKO] = mMKO;
//    mModules[ModuleCommands::OTD] = mOTD;
//    mModules[ModuleCommands::STM] = mSTM;
//    mModules[ModuleCommands::TECH] = mTech;
//    mModules[ModuleCommands::POWER_UNIT_BUP] = mPowerBUP;
//    mModules[ModuleCommands::POWER_UNIT_PNA] = mPowerPNA;

    // Start modules initialization. They will send 'initializationFinished' signal on initialization finish depending on its internal logic
    // The entire system will be initialized and ready to execute commands after 'initializationFinished' signal will be receceived from all modules
    //mPowerBUP->onApplicationStart();
    //mPowerPNA->onApplicationStart();
    //mOTD->onApplicationStart();
    //mSTM->onApplicationStart();
    //mTech->onApplicationStart();
    //mMKO->onApplicationStart();

    // Create templates for internal protocol commands parameters
    setupCommandsParams();
}

//void SystemState::setDefaultState()
//{
//    LOG_INFO("Setting default system state...");

//    foreach (AbstractModule* module, mModules.values())
//    {
//        LOG_INFO(QString("Module %1 state is %2").arg(module->moduleName()).arg(module->moduleState()));

//        if (module->moduleState() == AbstractModule::INITIALIZED_OK)
//        {
//            module->setDefaultState();
//        }
//    }
//}

QString SystemState::paramName(int module, int command, int param, bool isInputParam) const
{
    if (module >= 0 && module < ModuleCommands::MODULES_COUNT)
    {
        const QMap<int, QStringList>& container = (isInputParam ? mInParams[module] : mOutParams[module]);
        QMap<int, QStringList>::const_iterator it = container.find(command);
        if (it != container.end())
        {
            if (param >= 0 && param < it.value().size())
            {
                return it.value().at(param);
            }
        }
    }

    return "";
}

int SystemState::paramsCount(int module, int command, bool isInputParam) const
{
    if (module >= 0 && module < ModuleCommands::MODULES_COUNT)
    {
        const QMap<int, QStringList>& container = isInputParam ? mInParams[module] : mOutParams[module];
        QMap<int, QStringList>::const_iterator it = container.find(command);
        if (it != container.end())
        {
            return it.value().size();
        }
    }

    return 0;
}

void SystemState::setupCommandsParams()
{
    createPowerUnitCommandsParams();

    int TODO; // replace by constant usage

    {
        QMap<int, QStringList> inParams;
        QMap<int, QStringList> outParams;

        // commands input params
        QStringList sendTestArrayParams; // hardcoded with 0
        QStringList receiveTestArrayParams; // hardcoded with 0 checks on receive
        QStringList sendTestArrayForChannelParams; // hardcoded with 0
        QStringList receiveTestArrayForChannelParams; // hardcoded with 0 checks on receive
        QStringList enablePowerSupplyForAngleSensorParams; // auto set params (main of reserve kit)

        QStringList sendCommandArrayParams;
        sendCommandArrayParams.push_back(paramName(MODE_PSY));
        sendCommandArrayParams.push_back(paramName(STEPS_PSY));
        sendCommandArrayParams.push_back(paramName(VELOCITY_PSY));
        sendCommandArrayParams.push_back(paramName(CURRENT_PSY));
        sendCommandArrayParams.push_back(paramName(MODE_NU));
        sendCommandArrayParams.push_back(paramName(STEPS_NU));
        sendCommandArrayParams.push_back(paramName(VELOCITY_NU));
        sendCommandArrayParams.push_back(paramName(CURRENT_NU));

        QStringList sendCommandArrayForChannelParams;
        sendCommandArrayForChannelParams.push_back(paramName(MODE));
        sendCommandArrayForChannelParams.push_back(paramName(STEPS));
        sendCommandArrayForChannelParams.push_back(paramName(VELOCITY));
        sendCommandArrayForChannelParams.push_back(paramName(CURRENT));

        QStringList receiveCommandArrayParams;
        receiveCommandArrayParams.push_back(paramName(MODE_PSY));
        receiveCommandArrayParams.push_back(paramName(STEPS_PSY));
        receiveCommandArrayParams.push_back(paramName(VELOCITY_PSY));
        receiveCommandArrayParams.push_back(paramName(CURRENT_PSY));
        receiveCommandArrayParams.push_back(paramName(MODE_NU));
        receiveCommandArrayParams.push_back(paramName(STEPS_NU));
        receiveCommandArrayParams.push_back(paramName(VELOCITY_NU));
        receiveCommandArrayParams.push_back(paramName(CURRENT_NU));
        receiveCommandArrayParams.push_back(paramName(ANGLE_PSY));
        receiveCommandArrayParams.push_back(paramName(ANGLE_NU));
        receiveCommandArrayParams.push_back(paramName(SENSOR_FLAG));
        receiveCommandArrayParams.push_back(paramName(TEMPERATURE));

        QStringList receiveCommandArrayForChannelParams;
        sendCommandArrayForChannelParams.push_back(paramName(MODE));
        sendCommandArrayForChannelParams.push_back(paramName(STEPS));
        sendCommandArrayForChannelParams.push_back(paramName(VELOCITY));
        sendCommandArrayForChannelParams.push_back(paramName(CURRENT));

        inParams[ModuleMKO::SEND_TEST_ARRAY] = sendTestArrayParams;
        inParams[ModuleMKO::SEND_COMMAND_ARRAY] = sendCommandArrayParams;
        inParams[ModuleMKO::SEND_TEST_ARRAY_FOR_CHANNEL] = sendTestArrayForChannelParams;
        inParams[ModuleMKO::SEND_COMMAND_ARRAY_FOR_CHANNEL] = sendCommandArrayForChannelParams;
        inParams[ModuleMKO::SEND_TO_ANGLE_SENSOR] = enablePowerSupplyForAngleSensorParams;

        outParams[ModuleMKO::RECEIVE_TEST_ARRAY] = receiveTestArrayParams;
        outParams[ModuleMKO::RECEIVE_COMMAND_ARRAY] = receiveCommandArrayParams;
        outParams[ModuleMKO::RECEIVE_TEST_ARRAY_FOR_CHANNEL] = receiveTestArrayForChannelParams;
        outParams[ModuleMKO::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL] = receiveCommandArrayForChannelParams;

        mInParams[ModuleCommands::MKO] = inParams;
        mOutParams[ModuleCommands::MKO] = outParams;
    }

    // STM
    {
        //QMap<int, QStringList> params;
        //mInParams[ModuleCommands::STM] = params;

        // STM
        //addCommand(tr("Включить канал СТМ к БП"), ModuleCommands::POWER_CHANNEL_CTRL);
        //addCommand(tr("Проверить предохранители"), ModuleCommands::GET_PWR_MODULE_FUSE_STATE);
        //addCommand(tr("Получить телеметрию канала"), ModuleCommands::GET_CHANNEL_TELEMETRY);
        //addCommand(tr("Включить канал СТМ к МКО"), ModuleCommands::SET_MKO_PWR_CHANNEL_STATE);
        //addCommand(tr("Получить состояние канала СТМ к БП"), ModuleCommands::GET_POWER_MODULE_STATE);
        //addCommand(tr("Получить состояние канала СТМ к МКО"), ModuleCommands::GET_MKO_MODULE_STATE);
    }

    {
        QMap<int, QStringList> params;
        mInParams[ModuleCommands::TECH] = params;
    }



    /*
    GET_MODULE_ADDRESS              = 0x01,
    GET_STATUS_WORD                 = 0x02,
    RESET_ERROR                     = 0x03,
    SOFT_RESET                      = 0x04,
    //RESERVED_0x05 = 0x05,
    GET_SOWFTWARE_VER               = 0x06,
    ECHO                            = 0x07,
    //RESERVED_0x08 = 0x08,
    //RESERVED_0x09 = 0x09,
    //RESERVED_0x0A = 0x0A,
    POWER_CHANNEL_CTRL              = 0x0B, // Can be sent to STM only
    GET_PWR_MODULE_FUSE_STATE       = 0x0C, // Can be sent to STM only
    GET_CHANNEL_TELEMETRY           = 0x0D, // Can be sent to STM only
    SET_MKO_PWR_CHANNEL_STATE       = 0x0E, // Can be sent to STM only
    //MATRIX_CMD_CTRL                 = 0x0F, // Can be sent to MKU only
    SET_PACKET_SIZE_CAN             = 0x10, // Can be sent to TECH only
    ADD_BYTES_CAN                   = 0x11, // Can be sent to TECH only
    SEND_PACKET_CAN                 = 0x12, // Can be sent to TECH only
    CHECK_RECV_DATA_CAN             = 0x13, // Can be sent to TECH only
    RECV_DATA_CAN                   = 0x14, // Can be sent to TECH only
    CLEAN_BUFFER_CAN                = 0x15, // Can be sent to TECH only
    SET_PACKET_SIZE_RS485           = 0x16, // Can be sent to TECH only
    ADD_BYTES_RS485                 = 0x17, // Can be sent to TECH only
    SEND_PACKET_RS485               = 0x18, // Can be sent to TECH only
    CHECK_RECV_DATA_RS485           = 0x19, // Can be sent to TECH only
    RECV_DATA_RS485                 = 0x1A, // Can be sent to TECH only
    CLEAN_BUFFER_RS485              = 0x1B, // Can be sent to TECH only
    GET_TEMPERATURE_PT100           = 0x1C, // Can be sent to OTD only
    GET_DS1820_COUNT_LINE_1         = 0x1D, // Can be sent to OTD only (Psi)
    GET_DS1820_COUNT_LINE_2         = 0x1E, // Can be sent to OTD only (Nu)
    GET_TEMPERATURE_DS1820_LINE_1   = 0x1F, // Can be sent to OTD only (Psi)
    GET_TEMPERATURE_DS1820_LINE_2   = 0x20, // Can be sent to OTD only (Nu)
    GET_POWER_MODULE_STATE          = 0x21, // Can be sent to STM only
    //GET_MKU_MODULE_STATE            = 0x22, // Can be sent to MKU only
    GET_MKO_MODULE_STATE            = 0x23, // Can be sent to STM only
    SET_MODE_RS485                  = 0x24, // Can be sent to TECH only
    SET_SPEED_RS485                 = 0x25, // Can be sent to TECH only
    RESET_LINE_1                    = 0x26, // Can be sent to OTD only (Psi)
    RESET_LINE_2                    = 0x27, // Can be sent to OTD only (Nu)
    START_MEASUREMENT_LINE_1        = 0x28, // Can be sent to OTD only (Psi) 1-2 seconds to perform
    START_MEASUREMENT_LINE_2        = 0x29, // Can be sent to OTD only (Nu) 1-2 seconds to perform
    GET_DS1820_ADDR_LINE_1          = 0x2A, // Can be sent to OTD only (Psi)
    GET_DS1820_ADDR_LINE_2          = 0x2B, // Can be sent to OTD only (Nu)
    SET_TECH_INTERFACE              = 0x3B, // Can be sent to TECH only
    GET_TECH_INTERFACE              = 0x3C, // Can be sent to TECH only
    RECV_DATA_SSI                   = 0x3D, // Can be sent to TECH only

    // Third party modules commands (arbitrary) >>>>

    // Power unit modules commands (0xFF00-started)
    SET_VOLTAGE_AND_CURRENT         = 0xFF01,
    SET_MAX_VOLTAGE_AND_CURRENT     = 0xFF02,
    SET_POWER_STATE                 = 0xFF03,
    GET_CURRENT_VOLTAGE_AND_CURRENT = 0xFF04,
    */
}

void SystemState::createPowerUnitCommandsParams()
{
    QStringList powerParams;
    powerParams.push_back(paramName(VOLTAGE));
    powerParams.push_back(paramName(CURRENT));

    // commands input params
    QMap<int, QStringList> params;


    QStringList powerSetParams;
    powerSetParams.push_back(paramName(VOLTAGE));
    params[ModuleCommands::SET_VOLTAGE_AND_CURRENT] = powerSetParams;
    params[ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT] = powerParams;
    mInParams[ModuleCommands::POWER_UNIT_BUP] = params;
    mInParams[ModuleCommands::POWER_UNIT_PNA] = params;

    params.clear();
    params[ModuleCommands::GET_VOLTAGE_AND_CURRENT] = powerParams;
    mOutParams[ModuleCommands::POWER_UNIT_BUP] = params;
    mOutParams[ModuleCommands::POWER_UNIT_PNA] = params;
}

void SystemState::createOTDCommandsParams()
{
    QMap<int, QStringList> params;

    QStringList temperatureParams;

    // PT-100 params
    temperatureParams.clear();
    int ptCount = mOTD->ptCount();
    for (int i = 0; i < ptCount; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") + */paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_PT100] = temperatureParams;

    // DS1820 line 1 params
    temperatureParams.clear();
    int dsCount1 = mOTD->dsCount(ModuleOTD::PSY);
    for (int i = 0; i < dsCount1; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") +*/ paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1] = temperatureParams;

    // DS1820 line 2 params
    temperatureParams.clear();
    int dsCount2 = mOTD->dsCount(ModuleOTD::NU);
    for (int i = 0; i < dsCount2; ++i)
    {
        temperatureParams.push_back(/*QString::number(i + 1) + QString(". ") +*/ paramName(TEMPERATURE));
    }

    params[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2] = temperatureParams;

    mOutParams[ModuleCommands::OTD] = params;
}

void SystemState::sendCommand(CmdActionModule* command)
{
    mCurCommand = command;

    const QMap<QString, QVariant>& inputParams = command->inputParams();
    const QMap<QString, QVariant>& outputParams = command->outputParams();

    VariableController* vc = command->variableController();

    Transaction transaction;
    transaction.moduleID = uint32_t(command->module());
    transaction.commandID = uint32_t(command->operation());

    // input params -> [type, value]
    for (auto it = inputParams.begin(); it != inputParams.end(); ++it)
    {
        uint32_t type = uint32_t(paramID(it.key()));

        if (it.value().type() == QMetaType::QString) // input param is variable, get its current value
        {
            qreal value = vc->currentValue(it.value().toString());
            transaction.inputParams[type] = QVariant(value);
        }
        else //input param is direct value
        {
            transaction.inputParams[type] = it.value();
        }
    }

    // output params -> [type, vaiableName]
    for (auto it = outputParams.begin(); it != outputParams.end(); ++it)
    {
        uint32_t type = uint32_t(paramID(it.key()));
        transaction.outputParams[type] = it.value();
    }

    switch (command->module())
    {
    case ModuleCommands::POWER_UNIT_BUP:
        emit sendToPowerUnitBUP(transaction);
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        emit sendToPowerUnitPNA(transaction);
        break;
    case ModuleCommands::OTD:
        emit sendToOTD(transaction);
        break;
    case ModuleCommands::STM:
        emit sendToSTM(transaction);
        break;
    case ModuleCommands::MKO:
        emit sendToMKO(transaction);
        break;
    case ModuleCommands::TECH:
        emit sendToTech(transaction);
        break;

    default:
        LOG_ERROR(QString("Module not defined"));
        break;
    }
}

void SystemState::onExecutionFinished(uint32_t error)
{
    if (error != 0) // 0 - successful execution
    {
        QMetaEnum moduleEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();
        QMetaEnum commandEnum = QMetaEnum::fromType<ModuleCommands::CommandID>();

        LOG_ERROR("Command execution failed. Module:%s Command:%s Error:%s",
                  moduleEnum.valueToKey(mCurCommand->module()),
                  commandEnum.valueToKey(mCurCommand->operation()),
                  QString::number(error));
    }

    mCurCommand = Q_NULLPTR;
    emit commandFinished(error == 0);
}

void SystemState::processResponse(const Transaction& response)
{
    if (!mCurCommand)
    {
        LOG_ERROR("Unexpected response received");
        return;
    }

    VariableController* vc = mCurCommand->variableController();
    uint32_t paramsCount = response.outputParams.size();

    for (auto it = response.outputParams.begin(); it != response.outputParams.end(); ++it)
    {
        QList<QVariant> list = it.value().toList();
        if (list.size() != 2)
        {
            LOG_ERROR(QString("Invalid output param in response"));
            continue;
        }

        QString varName = list.at(0).toString();
        qreal value = list.at(1).toDouble();
        vc->setCurrentValue(varName, value);
    }

    if (paramsCount > 0)
    {
        vc->makeDataSnapshot();
    }

    uint32_t error = response.errorCode;

    // in case of power unit
    //if (error == 1) ui->err1->setText("Overvoltage protection!"); //TODO - ошибки установки на блоке питания, если 0 - ошибки нет
    //if (error == 2) ui->err1->setText("Overcurrent protection!");
    //if (error == 4) ui->err1->setText("Overpower protection!");
    //if (error == 8) ui->err1->setText("Overtemperature protection!");

    onExecutionFinished(error);
}

QString SystemState::paramName(ParamID param) const
{
    return mParamNames.value(param, "");
}

QString SystemState::paramDefaultVarName(ParamID param) const
{
    return mDefaultVariables.value(param, "");
}

QString SystemState::paramDefaultDesc(ParamID param) const
{
    return mDefaultDescriptions.value(param, "");
}

SystemState::ParamID SystemState::paramID(const QString& name) const
{
    return mParamNames.key(name, UNDEFINED);
}

//void SystemState::onModuleStateChanged(ModuleCommands::ModuleID id, AbstractModule::ModuleState from, AbstractModule::ModuleState to)
//{
//    QMetaEnum stateEnum = QMetaEnum::fromType<AbstractModule::ModuleState>();
//    AbstractModule* module = mModules.value(id);

//    if (from == AbstractModule::INITIALIZING)
//    {
//        if (to == AbstractModule::INITIALIZED_OK)
//        {
//            LOG_INFO(QString("%1 initializion SUCCESS").arg(module->moduleName()));
//            if (id == ModuleCommands::OTD)
//            {
//                createOTDCommandsParams();
//            }
//        }
//        else if (to == AbstractModule::INITIALIZED_FAILED)
//        {
//            LOG_ERROR(QString("%1 initialization FAILED! Error: %2").arg(module->moduleName()).arg(module->errorString()));
//        }
//        else
//        {
//            LOG_ERROR(QString("Unexpected %1 state changing from %2 to %3").arg(module->moduleName()).arg(stateEnum.valueToKey(from)).arg(stateEnum.valueToKey(to)));
//        }

//        // check all modules are started
//        bool allModulesStarted = true;
//        foreach (AbstractModule* m, mModules.values())
//        {
//            if (m->moduleState() == AbstractModule::INITIALIZED_FAILED || m->moduleState() == AbstractModule::INITIALIZED_OK)
//            {
//                continue;
//            }

//            allModulesStarted = false;
//            break;
//        }

//        // if all modules are started, set them to default state
//        if (allModulesStarted)
//        {
//            setDefaultState();
//        }
//    }
//}

//void SystemState::onCyclogramStart()
//{
//    mMKO->onCyclogramStart();
//}

bool SystemState::isImplicit(const QString &name) const
{
    ParamID param = paramID(name);

    switch (param)
    {
    case SUBADDRESS:
    case CHANNEL_ID:
    case POWER_STATE:
        return true;

    default:
        break;
    }

    return false;
}
