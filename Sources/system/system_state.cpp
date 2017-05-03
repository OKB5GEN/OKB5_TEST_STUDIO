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
            LOG_FATAL(QString("No system_config.xml found"));
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
    mMKO(Q_NULLPTR),
    mOTD(Q_NULLPTR),
    mSTM(Q_NULLPTR),
    mTech(Q_NULLPTR),
    mPowerBUP(Q_NULLPTR),
    mPowerPNA(Q_NULLPTR),
    mCurCommand(Q_NULLPTR)
{
    mParamNames[VOLTAGE] = tr("Voltage, V");
    mParamNames[CURRENT] = tr("Current, A");
    mParamNames[TEMPERATURE] = tr("Temperature, °C");
    mParamNames[DRIVE_MODE] = tr("Mode");
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
    mParamNames[DEVICE_CLASS] = tr("Device class");
    mParamNames[POWER] = tr("Power, W");
    mParamNames[STATUS_PHYSICAL] = tr("Phys.status");
    mParamNames[STATUS_LOGICAL] = tr("Logic.status");
    mParamNames[SUBADDRESS] = tr("Subaddress");
    mParamNames[POWER_STATE] = tr("Power State");
    mParamNames[CHANNEL_ID] = tr("Channel");
    mParamNames[RELAY_STATE] = tr("Relay state");
    mParamNames[MODULE_ADDRESS] = tr("Module address");
    mParamNames[MODULE_READY] = tr("Module ready flag");
    mParamNames[MODULE_AFTER_RESET] = tr("Module after reset flag");
    mParamNames[MODULE_HAS_ERRORS] = tr("Module has errors flag");

    mDefaultVariables[VOLTAGE] = "U";
    mDefaultVariables[CURRENT] = "I";
    mDefaultVariables[TEMPERATURE] = "T";
    mDefaultVariables[DRIVE_MODE] = "M";
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
    mDefaultVariables[STATUS_PHYSICAL] = "PActive";
    mDefaultVariables[STATUS_LOGICAL] = "LActive";
    mDefaultVariables[DEVICE_CLASS] = "Class";
    mDefaultVariables[POWER] = "Pow";
    mDefaultVariables[RELAY_STATE] = "IsOn";
    mDefaultVariables[MODULE_ADDRESS] = "Addr";
    mDefaultVariables[MODULE_READY] = tr("IsReady");
    mDefaultVariables[MODULE_AFTER_RESET] = tr("IsReset");
    mDefaultVariables[MODULE_HAS_ERRORS] = tr("HasErr");

    mDefaultDescriptions[VOLTAGE] = tr("Voltage, V");
    mDefaultDescriptions[CURRENT] = tr("Current, A");
    mDefaultDescriptions[TEMPERATURE] = tr("Temperature, °C");
    mDefaultDescriptions[DRIVE_MODE] = tr("Mode");
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
    mDefaultDescriptions[STATUS_PHYSICAL] = tr("Pysical module status. 1 - active, 0 - inactive");
    mDefaultDescriptions[STATUS_LOGICAL] = tr("Logical module status. 1 - enabled, 0 - disabled");
    mDefaultDescriptions[DEVICE_CLASS] = tr("Device Class");
    mDefaultDescriptions[POWER] = tr("Power, W");
    mDefaultDescriptions[RELAY_STATE] = tr("Relay state. 1 - on, 0 - off");
    mDefaultDescriptions[MODULE_ADDRESS] = tr("Module address");
    mDefaultDescriptions[MODULE_READY] = tr("Module ready flag. 1 - ready, 0 - not ready");
    mDefaultDescriptions[MODULE_AFTER_RESET] = tr("Module is after reset flag. 1 - after reset, 0 - not after reset");
    mDefaultDescriptions[MODULE_HAS_ERRORS] = tr("Module error flag. 1 - module has errors, 0 - module has no errors");
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

    QString mode = emulatorEnabled ? tr("EMULATOR") : tr("DEVICE");
    LOG_INFO(QString("System mode: %1").arg(mode));

    // Create modules objects
    mMKO = new ModuleMKO(this);
    mMKO->setEmulator(emulatorEnabled);
    mMKO->setModuleID(ModuleCommands::MKO);
    connect(this, SIGNAL(sendToMKO(const Transaction&)), mMKO, SLOT(processCommand(const Transaction&)));
    connect(mMKO, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    mOTD = new ModuleOTD(this);
    mOTD->setEmulator(emulatorEnabled);
    mOTD->setModuleID(ModuleCommands::OTD);
    mOTD->setId(modules.value(ModuleCommands::OTD));
    connect(this, SIGNAL(sendToOTD(const Transaction&)), mOTD, SLOT(processCommand(const Transaction&)));
    connect(mOTD, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    connect(mOTD, SIGNAL(powerRelayStateChanged(ModuleCommands::PowerSupplyChannelID, ModuleCommands::PowerState)), mMKO, SLOT(onPowerRelayStateChanged(ModuleCommands::PowerSupplyChannelID, ModuleCommands::PowerState)));
    connect(mOTD, SIGNAL(powerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID, ModuleCommands::PowerState)), mMKO, SLOT(onPowerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID, ModuleCommands::PowerState)));

    mSTM = new ModuleSTM(this);
    mSTM->setEmulator(emulatorEnabled);
    mSTM->setModuleID(ModuleCommands::STM);
    mSTM->setId(modules.value(ModuleCommands::STM));
    connect(this, SIGNAL(sendToSTM(const Transaction&)), mSTM, SLOT(processCommand(const Transaction&)));
    connect(mSTM, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    mTech = new ModuleTech(this);
    mTech->setEmulator(emulatorEnabled);
    mTech->setModuleID(ModuleCommands::TECH);
    mTech->setId(modules.value(ModuleCommands::TECH));
    connect(this, SIGNAL(sendToTech(const Transaction&)), mTech, SLOT(processCommand(const Transaction&)));
    connect(mTech, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    mPowerBUP = new ModulePower(this);
    mPowerBUP->setEmulator(emulatorEnabled);
    mPowerBUP->setModuleID(ModuleCommands::POWER_UNIT_BUP);
    mPowerBUP->setId(modules.value(ModuleCommands::POWER_UNIT_BUP));
    connect(this, SIGNAL(sendToPowerUnitBUP(const Transaction&)), mPowerBUP, SLOT(processCommand(const Transaction&)));
    connect(mPowerBUP, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    mPowerPNA = new ModulePower(this);
    mPowerPNA->setEmulator(emulatorEnabled);
    mPowerPNA->setModuleID(ModuleCommands::POWER_UNIT_PNA);
    mPowerPNA->setId(modules.value(ModuleCommands::POWER_UNIT_PNA));
    connect(this, SIGNAL(sendToPowerUnitPNA(const Transaction&)), mPowerPNA, SLOT(processCommand(const Transaction&)));
    connect(mPowerPNA, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    // Create commands params list
    createPowerUnitCommandsParams();
    createMKOCommandsParams();
    createOTDCommandsParams();
    createTechCommandsParams();
    createSTMCommandsParams();
}

void SystemState::onApplicationFinish()
{
    mMKO->onApplicationFinish();
    mOTD->onApplicationFinish();
    mSTM->onApplicationFinish();
    mTech->onApplicationFinish();
    mPowerBUP->onApplicationFinish();
    mPowerPNA->onApplicationFinish();
}

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

void SystemState::createPowerUnitCommandsParams()
{
    QStringList getStatusParams;
    getStatusParams.append(paramName(STATUS_PHYSICAL));
    getStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList setStatusParams;
    setStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList powerParams;
    powerParams.append(paramName(VOLTAGE));
    powerParams.append(paramName(CURRENT));

    // commands input params
    QMap<int, QStringList> inParams;

    QStringList voltageSetParams;
    voltageSetParams.push_back(paramName(VOLTAGE));

    QStringList currentSetParams;
    currentSetParams.push_back(paramName(CURRENT));

    inParams[ModuleCommands::SET_VOLTAGE_AND_CURRENT] = voltageSetParams;
    inParams[ModuleCommands::SET_MODULE_LOGIC_STATUS] = setStatusParams;
    inParams[ModuleCommands::SET_OVP_THRESHOLD] = voltageSetParams;
    inParams[ModuleCommands::SET_OCP_THRESHOLD] = currentSetParams;

    mInParams[ModuleCommands::POWER_UNIT_BUP] = inParams;
    mInParams[ModuleCommands::POWER_UNIT_PNA] = inParams;

    // commands output params
    QMap<int, QStringList> outParams;

    QStringList deviceClass;
    deviceClass.push_back(paramName(DEVICE_CLASS));

    QStringList nomPowerParams;
    nomPowerParams.push_back(paramName(POWER));

    outParams[ModuleCommands::GET_VOLTAGE_AND_CURRENT] = powerParams;
    outParams[ModuleCommands::GET_MODULE_STATUS] = getStatusParams;
    outParams[ModuleCommands::GET_DEVICE_CLASS] = deviceClass;
    outParams[ModuleCommands::GET_NOMINAL_CURRENT] = currentSetParams;
    outParams[ModuleCommands::GET_NOMINAL_VOLTAGE] = voltageSetParams;
    outParams[ModuleCommands::GET_NOMINAL_POWER] = nomPowerParams;
    outParams[ModuleCommands::GET_OVP_THRESHOLD] = voltageSetParams;
    outParams[ModuleCommands::GET_OCP_THRESHOLD] = currentSetParams;

    mOutParams[ModuleCommands::POWER_UNIT_BUP] = outParams;
    mOutParams[ModuleCommands::POWER_UNIT_PNA] = outParams;
}

void SystemState::createMKOCommandsParams()
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
    sendCommandArrayForChannelParams.push_back(paramName(DRIVE_MODE));
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
    sendCommandArrayForChannelParams.push_back(paramName(DRIVE_MODE));
    sendCommandArrayForChannelParams.push_back(paramName(STEPS));
    sendCommandArrayForChannelParams.push_back(paramName(VELOCITY));
    sendCommandArrayForChannelParams.push_back(paramName(CURRENT));

    QStringList getStatusParams;
    getStatusParams.append(paramName(STATUS_PHYSICAL));
    getStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList setStatusParams;
    setStatusParams.append(paramName(STATUS_LOGICAL));

    inParams[ModuleCommands::SEND_TEST_ARRAY] = sendTestArrayParams;
    inParams[ModuleCommands::SEND_COMMAND_ARRAY] = sendCommandArrayParams;
    inParams[ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL] = sendTestArrayForChannelParams;
    inParams[ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL] = sendCommandArrayForChannelParams;
    inParams[ModuleCommands::SEND_TO_ANGLE_SENSOR] = enablePowerSupplyForAngleSensorParams;
    inParams[ModuleCommands::SET_MODULE_LOGIC_STATUS] = setStatusParams;

    outParams[ModuleCommands::RECEIVE_TEST_ARRAY] = receiveTestArrayParams;
    outParams[ModuleCommands::RECEIVE_COMMAND_ARRAY] = receiveCommandArrayParams;
    outParams[ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL] = receiveTestArrayForChannelParams;
    outParams[ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL] = receiveCommandArrayForChannelParams;
    outParams[ModuleCommands::GET_MODULE_STATUS] = getStatusParams;

    mInParams[ModuleCommands::MKO] = inParams;
    mOutParams[ModuleCommands::MKO] = outParams;
}

void SystemState::createOTDCommandsParams()
{
    QStringList getStatusParams;
    getStatusParams.append(paramName(STATUS_PHYSICAL));
    getStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList setStatusParams;
    setStatusParams.append(paramName(STATUS_LOGICAL));

    // input params
    QMap<int, QStringList> inParams;
    inParams[ModuleCommands::SET_MODULE_LOGIC_STATUS] = setStatusParams;

    mInParams[ModuleCommands::OTD] = inParams;

    // output params
    QStringList moduleAddressParams;
    moduleAddressParams.append(paramName(MODULE_ADDRESS));

    QStringList statusWordParams;
    statusWordParams.append(paramName(MODULE_READY));
    statusWordParams.append(paramName(MODULE_AFTER_RESET));
    statusWordParams.append(paramName(MODULE_HAS_ERRORS));

    int TODO; // solve problem with sensors count data
    QMap<int, QStringList> outParams;

    QStringList temperatureParams;

    // PT-100 params
    temperatureParams.clear();
    int ptCount = mOTD->ptCount();
    for (int i = 0; i < ptCount; ++i)
    {
        temperatureParams.push_back(paramName(TEMPERATURE));
    }

    outParams[ModuleCommands::GET_TEMPERATURE_PT100] = temperatureParams;

    // DS1820 line 1 params
    temperatureParams.clear();
    int dsCount1 = mOTD->dsCount(ModuleOTD::PSY);
    for (int i = 0; i < dsCount1; ++i)
    {
        temperatureParams.push_back(paramName(TEMPERATURE));
    }

    outParams[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1] = temperatureParams;

    // DS1820 line 2 params
    temperatureParams.clear();
    int dsCount2 = mOTD->dsCount(ModuleOTD::NU);
    for (int i = 0; i < dsCount2; ++i)
    {
        temperatureParams.push_back(paramName(TEMPERATURE));
    }

    outParams[ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2] = temperatureParams;
    outParams[ModuleCommands::GET_MODULE_STATUS] = getStatusParams;
    outParams[ModuleCommands::GET_MODULE_ADDRESS] = moduleAddressParams;
    outParams[ModuleCommands::GET_STATUS_WORD] = statusWordParams;

    mOutParams[ModuleCommands::OTD] = outParams;
}

void SystemState::createSTMCommandsParams()
{
    QStringList getStatusParams;
    getStatusParams.append(paramName(STATUS_PHYSICAL));
    getStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList setStatusParams;
    setStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList relayStateParams;
    relayStateParams.append(paramName(RELAY_STATE));

    // input params
    QMap<int, QStringList> inParams;
    inParams[ModuleCommands::SET_MODULE_LOGIC_STATUS] = setStatusParams;

    mInParams[ModuleCommands::STM] = inParams;

    QStringList moduleAddressParams;
    moduleAddressParams.append(paramName(MODULE_ADDRESS));

    QStringList statusWordParams;
    statusWordParams.append(paramName(MODULE_READY));
    statusWordParams.append(paramName(MODULE_AFTER_RESET));
    statusWordParams.append(paramName(MODULE_HAS_ERRORS));

    // output params
    QMap<int, QStringList> outParams;
    outParams[ModuleCommands::GET_MODULE_STATUS] = getStatusParams;
    outParams[ModuleCommands::GET_POWER_CHANNEL_STATE] = relayStateParams;
    outParams[ModuleCommands::GET_MKO_POWER_CHANNEL_STATE] = relayStateParams;
    outParams[ModuleCommands::GET_MODULE_ADDRESS] = moduleAddressParams;
    outParams[ModuleCommands::GET_STATUS_WORD] = statusWordParams;

    mOutParams[ModuleCommands::STM] = outParams;
}

void SystemState::createTechCommandsParams()
{
    QStringList getStatusParams;
    getStatusParams.append(paramName(STATUS_PHYSICAL));
    getStatusParams.append(paramName(STATUS_LOGICAL));

    QStringList setStatusParams;
    setStatusParams.append(paramName(STATUS_LOGICAL));

    // input params
    QMap<int, QStringList> inParams;
    inParams[ModuleCommands::SET_MODULE_LOGIC_STATUS] = setStatusParams;

    mInParams[ModuleCommands::TECH] = inParams;

    QStringList moduleAddressParams;
    moduleAddressParams.append(paramName(MODULE_ADDRESS));

    QStringList statusWordParams;
    statusWordParams.append(paramName(MODULE_READY));
    statusWordParams.append(paramName(MODULE_AFTER_RESET));
    statusWordParams.append(paramName(MODULE_HAS_ERRORS));
    //statusWordParams.append(paramName(CUSTOM_ERROR)); // Tech module has some custom error codes in status word

    // output params
    QMap<int, QStringList> outParams;
    outParams[ModuleCommands::GET_MODULE_STATUS] = getStatusParams;
    outParams[ModuleCommands::GET_MODULE_ADDRESS] = moduleAddressParams;
    outParams[ModuleCommands::GET_STATUS_WORD] = statusWordParams;

    mOutParams[ModuleCommands::TECH] = outParams;
}

void SystemState::sendCommand(CmdActionModule* command)
{
    mCurCommand = command;

    // 1. Convert module command data to transaction
    const QMap<QString, QVariant>& inputParams = command->inputParams();
    const QMap<QString, QVariant>& outputParams = command->outputParams();

    VariableController* vc = command->variableController();

    Transaction transaction;
    transaction.moduleID = uint32_t(command->module());
    transaction.commandID = command->operation();

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

    // 2. Check and process command if it is "local" (does not need any remote interaction with module)
    if (processLocalCommand(transaction))
    {
        mCurCommand = Q_NULLPTR;
        emit commandFinished(transaction.error.isEmpty());
        return;
    }

    // 3. If command is not local, check module status.
    // If module is not ready (physically or logically disconnected), skip command processing with error
    AbstractModule* module = moduleByID(command->module());
    if (module)
    {
        QString error;
        if (!module->isPhysicallyActive())
        {
            error = QString("physically");
        }

        if (!module->isLogicallyActive())
        {
            if (!error.isEmpty())
            {
                error += QString(" and ");
            }

            error = QString("logically");
        }

        if (!error.isEmpty())
        {
            QMetaEnum commandEnum = QMetaEnum::fromType<ModuleCommands::CommandID>();
            QString cmdName = QString(commandEnum.valueToKey(command->operation()));

            LOG_ERROR(QString("%1 can not process %2 command, because module is %3 INACTIVE").arg(module->moduleName()).arg(cmdName).arg(error));
            mCurCommand = Q_NULLPTR;
            emit commandFinished(false);
            return;
        }
    }

    // 4. If module is ready to process commands, send transaction to module
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
        {
            LOG_ERROR(QString("Module not defined"));
            mCurCommand = Q_NULLPTR;
            emit commandFinished(true);
            return;
        }
        break;
    }

    int TODO; // start protection timer (for cases when some module logics is buggy) - 10-20-30 seconds for example
}

AbstractModule* SystemState::moduleByID(ModuleCommands::ModuleID moduleID) const
{
    AbstractModule* module = Q_NULLPTR;

    switch (moduleID)
    {
    case ModuleCommands::POWER_UNIT_BUP:
        module = mPowerBUP;
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        module = mPowerPNA;
        break;
    case ModuleCommands::OTD:
        module = mOTD;
        break;
    case ModuleCommands::STM:
        module = mSTM;
        break;
    case ModuleCommands::MKO:
        module = mMKO;
        break;
    case ModuleCommands::TECH:
        module = mTech;
        break;
    default:
        break;
    }

    return module;
}

bool SystemState::processLocalCommand(Transaction& transaction)
{
    QString error;
    AbstractModule* module = moduleByID(ModuleCommands::ModuleID(transaction.moduleID));

    if (!module)
    {
        error = QString("Unknown module id=%1").arg(transaction.moduleID);
    }

    switch (transaction.commandID)
    {
        case ModuleCommands::GET_MODULE_STATUS:
        {
            QString physVar = transaction.outputParams.value(STATUS_PHYSICAL).toString();
            QString logVar = transaction.outputParams.value(STATUS_LOGICAL).toString();
            qreal physValue = 0;
            qreal logValue = 0;

            VariableController* vc = mCurCommand->variableController();

            if (module)
            {
                physValue = module->isPhysicallyActive() ? 1 : 0;
                logValue = module->isLogicallyActive() ? 1 : 0;
            }

            vc->setCurrentValue(physVar, physValue);
            vc->setCurrentValue(logVar, logValue);
        }
        break;

    case ModuleCommands::SET_MODULE_LOGIC_STATUS:
        {
            qint64 logValue = qRound64(transaction.inputParams.value(STATUS_LOGICAL).toDouble());

            if (module)
            {
                LOG_INFO(QString("Set module %1 logically %2").arg(module->moduleName()).arg((logValue != 0) ? "ACTIVE" : "INACTIVE"));
                module->setLogicallyActive(logValue != 0);
            }
        }
        break;

    default:
        return false;
        break;
    }

    return true;
}

void SystemState::processResponse(const Transaction& response)
{
    if (!mCurCommand)
    {
        LOG_ERROR(QString("Unexpected response received"));
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

    QString error = response.error;
    if (!error.isEmpty())
    {
        QMetaEnum moduleEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();
        QMetaEnum commandEnum = QMetaEnum::fromType<ModuleCommands::CommandID>();
        QString module = moduleEnum.valueToKey(mCurCommand->module());
        QString command = commandEnum.valueToKey(mCurCommand->operation());

        LOG_ERROR(QString("Command execution failed. Module:%1, Command:%2, Error:%3").arg(module).arg(command).arg(error));
    }

    int TODO; // stop protection timer

    mCurCommand = Q_NULLPTR;
    emit commandFinished(error.isEmpty());
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

bool SystemState::isImplicit(const QString &name) const
{
    ParamID param = paramID(name);

    switch (param)
    {
    case SUBADDRESS:
    case CHANNEL_ID:
    case POWER_STATE:
        return true; //TODO RELAY_STATE?

    default:
        break;
    }

    return false;
}
