#include "Headers/system/system_state.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_drive_simulator.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/app_settings.h"
#include "Headers/logger/Logger.h"

#include <QMetaEnum>
#include <QtSerialPort>
#include <QTimer>

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
            if (xml.name() == "system_config" && xml.attributes().value("version") == "1.0") //TODO file versioning
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
    mDS(Q_NULLPTR),
    mSTM(Q_NULLPTR),
    mTech(Q_NULLPTR),
    mPowerBUP(Q_NULLPTR),
    mPowerPNA(Q_NULLPTR),
    mCurCommand(Q_NULLPTR)
{
    mProtectionTimer = new QTimer(this);
    mProtectionTimer->setSingleShot(true);
    connect(mProtectionTimer, SIGNAL(timeout()), this, SLOT(onResponseTimeout()));

    updateParams();
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

    mDS = new ModuleDriveSimulator(this);
    mDS->setEmulator(emulatorEnabled);
    mDS->setModuleID(ModuleCommands::DRIVE_SIMULATOR);
    mDS->setId(modules.value(ModuleCommands::DRIVE_SIMULATOR));
    connect(this, SIGNAL(sendToDS(const Transaction&)), mDS, SLOT(processCommand(const Transaction&)));
    connect(mDS, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    mSTM = new ModuleSTM(this);
    mSTM->setEmulator(emulatorEnabled);
    mSTM->setModuleID(ModuleCommands::STM);
    mSTM->setId(modules.value(ModuleCommands::STM));
    connect(this, SIGNAL(sendToSTM(const Transaction&)), mSTM, SLOT(processCommand(const Transaction&)));
    connect(mSTM, SIGNAL(commandResult(const Transaction&)), this, SLOT(processResponse(const Transaction&)));

    connect(mSTM, SIGNAL(powerRelayStateChanged(ModuleCommands::PowerSupplyChannelID, ModuleCommands::PowerState)), mMKO, SLOT(onPowerRelayStateChanged(ModuleCommands::PowerSupplyChannelID, ModuleCommands::PowerState)));
    connect(mSTM, SIGNAL(powerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID, ModuleCommands::PowerState)), mMKO, SLOT(onPowerMKORelayStateChanged(ModuleCommands::MKOPowerSupplyChannelID, ModuleCommands::PowerState)));

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
    createDSCommandsParams();
    createTechCommandsParams();
    createSTMCommandsParams();
}

void SystemState::onApplicationFinish()
{
    mMKO->onApplicationFinish();
    mOTD->onApplicationFinish();
    mDS->onApplicationFinish();
    mSTM->onApplicationFinish();
    mTech->onApplicationFinish();
    mPowerBUP->onApplicationFinish();
    mPowerPNA->onApplicationFinish();
}

SystemState::Param SystemState::paramData(ParamID param) const
{
    auto it = mParams.find(param);

    if (it != mParams.end())
    {
        return it.value();
    }

    Param defaultParam;
    defaultParam.name = "UNKNOWN";
    defaultParam.variable = "UNKNOWN";
    return defaultParam;
}

void SystemState::addCommand(uint32_t id, std::initializer_list<uint32_t> input, std::initializer_list<uint32_t> output, QMap<uint32_t, Command>* commands)
{
    if (!commands)
    {
        return;
    }

    Command command;
    command.inputParams = QSet<uint32_t>(input);
    command.outputParams = QSet<uint32_t>(output);
    commands->insert(id, command);
}

void SystemState::createPowerUnitCommandsParams()
{
    QMap<uint32_t, Command> commands;

    addCommand(ModuleCommands::GET_MODULE_STATUS,           {},                 {STATUS_PHYSICAL, STATUS_LOGICAL},  &commands);
    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,     {STATUS_LOGICAL},   {},                                 &commands);
    addCommand(ModuleCommands::GET_VOLTAGE_AND_CURRENT,     {},                 {VOLTAGE, CURRENT},                 &commands);
    addCommand(ModuleCommands::SET_VOLTAGE_AND_CURRENT,     {VOLTAGE},          {},                                 &commands);
    addCommand(ModuleCommands::SET_OVP_THRESHOLD,           {VOLTAGE},          {},                                 &commands);
    addCommand(ModuleCommands::GET_NOMINAL_VOLTAGE,         {},                 {VOLTAGE},                          &commands);
    addCommand(ModuleCommands::GET_OVP_THRESHOLD,           {},                 {VOLTAGE},                          &commands);
    addCommand(ModuleCommands::SET_OCP_THRESHOLD,           {CURRENT},          {},                                 &commands);
    addCommand(ModuleCommands::GET_NOMINAL_CURRENT,         {},                 {CURRENT},                          &commands);
    addCommand(ModuleCommands::GET_OCP_THRESHOLD,           {},                 {CURRENT},                          &commands);
    addCommand(ModuleCommands::GET_FUSE_STATE,              {FUSE_ID},          {FUSE_STATE},                       &commands);
    addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE,     {},                 {RELAY_STATE},                      &commands);
    addCommand(ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, {},                 {RELAY_STATE},                      &commands);
    addCommand(ModuleCommands::GET_DEVICE_CLASS,            {},                 {DEVICE_CLASS},                     &commands);
    addCommand(ModuleCommands::GET_NOMINAL_POWER,           {},                 {POWER},                            &commands);

    mCommands[ModuleCommands::POWER_UNIT_BUP] = commands;
    mCommands[ModuleCommands::POWER_UNIT_PNA] = commands;
}

void SystemState::createMKOCommandsParams()
{
    QMap<uint32_t, Command> commands;

    std::initializer_list<uint32_t> sendCommandArray = {MODE_PSY, STEPS_PSY, VELOCITY_PSY, CURRENT_PSY, MODE_NU, STEPS_NU, VELOCITY_NU, CURRENT_NU};
    std::initializer_list<uint32_t> receiveCommandArray = {MODE_PSY, STEPS_PSY, VELOCITY_PSY, CURRENT_PSY, MODE_NU, STEPS_NU, VELOCITY_NU, CURRENT_NU, ANGLE_PSY, ANGLE_NU, SENSOR_FLAG, TEMPERATURE};
    std::initializer_list<uint32_t> commandArrayForChannel = {DRIVE_MODE, STEPS, VELOCITY, CURRENT};

    addCommand(ModuleCommands::SEND_TEST_ARRAY,                     {},                         {},                                 &commands);
    addCommand(ModuleCommands::SEND_COMMAND_ARRAY,                  sendCommandArray,           {},                                 &commands);
    addCommand(ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL,         {},                         {},                                 &commands);
    addCommand(ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL,      commandArrayForChannel,     {},                                 &commands);
    addCommand(ModuleCommands::SEND_TO_ANGLE_SENSOR,                {},                         {},                                 &commands);
    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,             {STATUS_LOGICAL},           {},                                 &commands);
    addCommand(ModuleCommands::RECEIVE_TEST_ARRAY,                  {},                         {},                                 &commands);
    addCommand(ModuleCommands::RECEIVE_COMMAND_ARRAY,               {},                         receiveCommandArray,                &commands);
    addCommand(ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL,      {},                         {},                                 &commands);
    addCommand(ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL,   {},                         commandArrayForChannel,             &commands);
    addCommand(ModuleCommands::GET_MODULE_STATUS,                   {},                         {STATUS_PHYSICAL, STATUS_LOGICAL},  &commands);
    addCommand(ModuleCommands::GET_MKO_POWER_CHANNEL_STATE,         {},                         {RELAY_STATE},                      &commands);

    mCommands[ModuleCommands::MKO] = commands;
}

void SystemState::createOTDCommandsParams()
{
    QMap<uint32_t, Command> commands;

    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,         {STATUS_LOGICAL},   {},                                                     &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1,   {SENSOR_NUMBER},    {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2,   {SENSOR_NUMBER},    {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_PT100,           {},                 {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_MODULE_STATUS,               {},                 {STATUS_PHYSICAL, STATUS_LOGICAL},                      &commands);
    addCommand(ModuleCommands::GET_MODULE_ADDRESS,              {},                 {MODULE_ADDRESS},                                       &commands);
    addCommand(ModuleCommands::GET_STATUS_WORD,                 {},                 {MODULE_READY, MODULE_AFTER_RESET, MODULE_HAS_ERRORS},  &commands);
    addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_1,         {},                 {SENSORS_COUNT},                                        &commands);
    addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_2,         {},                 {SENSORS_COUNT},                                        &commands);

    mCommands[ModuleCommands::OTD] = commands;
}

void SystemState::createDSCommandsParams()
{
    QMap<uint32_t, Command> commands;

    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,         {STATUS_LOGICAL},   {},                                                     &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1,   {SENSOR_NUMBER},    {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_MODULE_STATUS,               {},                 {STATUS_PHYSICAL, STATUS_LOGICAL},                      &commands);
    addCommand(ModuleCommands::GET_MODULE_ADDRESS,              {},                 {MODULE_ADDRESS},                                       &commands);
    addCommand(ModuleCommands::GET_STATUS_WORD,                 {},                 {MODULE_READY, MODULE_AFTER_RESET, MODULE_HAS_ERRORS},  &commands);
    addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_1,         {},                 {SENSORS_COUNT},                                        &commands);

    mCommands[ModuleCommands::DRIVE_SIMULATOR] = commands;
}

void SystemState::createSTMCommandsParams()
{
    QMap<uint32_t, Command> commands;

    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,     {STATUS_LOGICAL},   {},                                                     &commands);
    addCommand(ModuleCommands::GET_FUSE_STATE,              {FUSE_ID},          {FUSE_STATE},                                           &commands);
    addCommand(ModuleCommands::GET_CHANNEL_TELEMETRY,       {CHANNEL_ID},       {VOLTAGE},                                              &commands);
    addCommand(ModuleCommands::GET_MODULE_STATUS,           {},                 {STATUS_PHYSICAL, STATUS_LOGICAL},                      &commands);
    addCommand(ModuleCommands::GET_MODULE_ADDRESS,          {},                 {MODULE_ADDRESS},                                       &commands);
    addCommand(ModuleCommands::GET_STATUS_WORD,             {},                 {MODULE_READY, MODULE_AFTER_RESET, MODULE_HAS_ERRORS},  &commands);

    // TODO: moved to power units and MKO
    addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE,     {},                 {RELAY_STATE},                                          &commands);
    addCommand(ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, {},                 {RELAY_STATE},                                          &commands);

    mCommands[ModuleCommands::STM] = commands;
}

void SystemState::createTechCommandsParams()
{
    QMap<uint32_t, Command> commands;

    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS,         {STATUS_LOGICAL},   {},                                                     &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1,   {SENSOR_NUMBER},    {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2,   {SENSOR_NUMBER},    {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_TEMPERATURE_PT100,           {},                 {TEMPERATURE},                                          &commands);
    addCommand(ModuleCommands::GET_MODULE_STATUS,               {},                 {STATUS_PHYSICAL, STATUS_LOGICAL},                      &commands);
    addCommand(ModuleCommands::GET_MODULE_ADDRESS,              {},                 {MODULE_ADDRESS},                                       &commands);
    addCommand(ModuleCommands::GET_STATUS_WORD,                 {},                 {MODULE_READY, MODULE_AFTER_RESET, MODULE_HAS_ERRORS},  &commands);
    addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_1,         {},                 {SENSORS_COUNT},                                        &commands);
    addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_2,         {},                 {SENSORS_COUNT},                                        &commands);

    mCommands[ModuleCommands::TECH] = commands;
}

void SystemState::sendCommand(CmdActionModule* command)
{
    mCurCommand = command;

    // 1. Convert module command data to transaction
    const QMap<uint32_t, QVariant>& inputParams = command->inputParams();
    const QMap<uint32_t, QVariant>& outputParams = command->outputParams();

    VariableController* vc = command->variableController();

    Transaction transaction;
    transaction.moduleID = uint32_t(command->module());
    transaction.commandID = command->operation();

    // input params -> [type, value]
    for (auto it = inputParams.begin(); it != inputParams.end(); ++it)
    {
        uint32_t type = it.key();

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
        uint32_t type = it.key();
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
        {
            switch (command->operation()) // hack for moving STM commands to Power Unit/MKO commands
            {
            case ModuleCommands::SET_POWER_CHANNEL_STATE:
            case ModuleCommands::GET_POWER_CHANNEL_STATE:
            case ModuleCommands::GET_FUSE_STATE:
                emit sendToSTM(transaction);
                break;

            default: // by default send command to power unit
                emit sendToPowerUnitBUP(transaction);
                break;
            }
        }
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        {
            switch (command->operation()) // hack for moving STM commands to Power Unit/MKO commands
            {
            case ModuleCommands::SET_POWER_CHANNEL_STATE:
            case ModuleCommands::GET_POWER_CHANNEL_STATE:
            case ModuleCommands::GET_FUSE_STATE:
                emit sendToSTM(transaction);
                break;

            default: // by default send command to power unit
                emit sendToPowerUnitPNA(transaction);
                break;
            }
        }
        break;
    case ModuleCommands::OTD:
        emit sendToOTD(transaction);
        break;
    case ModuleCommands::DRIVE_SIMULATOR:
        emit sendToDS(transaction);
        break;
    case ModuleCommands::STM:
        emit sendToSTM(transaction);
        break;
    case ModuleCommands::MKO:
        {
            switch (command->operation()) // hack for moving STM commands to Power Unit/MKO commands
            {
            case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
            case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
                emit sendToSTM(transaction);
                break;

            default: // by default send command to MKO
                emit sendToMKO(transaction);
                break;
            }
        }
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

    int protectionTimeout = AppSettings::instance().settingValue(AppSettings::MODULE_RESPONSE_WAIT_TIMEOUT).toInt();
    mProtectionTimer->start(protectionTimeout);
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
    case ModuleCommands::DRIVE_SIMULATOR:
        module = mDS;
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

void SystemState::stop()
{
    LOG_INFO("System state stopped");

    mProtectionTimer->stop();
    mCurCommand = Q_NULLPTR;
}

void SystemState::processResponse(const Transaction& response)
{
    mProtectionTimer->stop();

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

    mCurCommand = Q_NULLPTR;
    emit commandFinished(error.isEmpty());
}

void SystemState::onResponseTimeout()
{
    if (!mCurCommand)
    {
        LOG_ERROR(QString("No command send but protection timer timeout occured"));
        return;
    }

    QMetaEnum moduleEnum = QMetaEnum::fromType<ModuleCommands::ModuleID>();
    QMetaEnum commandEnum = QMetaEnum::fromType<ModuleCommands::CommandID>();
    QString module = moduleEnum.valueToKey(mCurCommand->module());
    QString command = commandEnum.valueToKey(mCurCommand->operation());

    mCurCommand->stop();
    LOG_ERROR(QString("Command execution failed. Module:%1, Command:%2. Response timeout").arg(module).arg(command));
    emit commandFinished(false);
}

bool SystemState::isImplicit(ParamID param) const
{
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

bool SystemState::isSetter(ModuleCommands::CommandID command)
{
    switch (command)
    {
    //case ModuleCommands::GET_MODULE_ADDRESS:
    //case ModuleCommands::GET_STATUS_WORD:
    case ModuleCommands::RESET_ERROR:
    case ModuleCommands::SOFT_RESET:
    //case ModuleCommands::GET_SOWFTWARE_VERSION:
    case ModuleCommands::ECHO:
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
    //case ModuleCommands::GET_FUSE_STATE:
    //case ModuleCommands::GET_CHANNEL_TELEMETRY:
    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
    case ModuleCommands::SET_PACKET_SIZE_CAN:
    case ModuleCommands::ADD_BYTES_CAN:
    case ModuleCommands::SEND_PACKET_CAN:
    //case ModuleCommands::CHECK_RECV_DATA_CAN:
    case ModuleCommands::RECV_DATA_CAN:
    case ModuleCommands::CLEAN_BUFFER_CAN:
    case ModuleCommands::SET_PACKET_SIZE_RS485:
    case ModuleCommands::ADD_BYTES_RS485:
    case ModuleCommands::SEND_PACKET_RS485:
    //case ModuleCommands::CHECK_RECV_DATA_RS485:
    case ModuleCommands::RECV_DATA_RS485:
    case ModuleCommands::CLEAN_BUFFER_RS485:
    //case ModuleCommands::GET_TEMPERATURE_PT100:
    //case ModuleCommands::GET_DS1820_COUNT_LINE_1:
    //case ModuleCommands::GET_DS1820_COUNT_LINE_2:
    //case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
    //case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
    //case ModuleCommands::GET_POWER_CHANNEL_STATE:
    //case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
    case ModuleCommands::SET_MODE_RS485:
    case ModuleCommands::SET_SPEED_RS485:
    case ModuleCommands::RESET_LINE_1:
    case ModuleCommands::RESET_LINE_2:
    case ModuleCommands::START_MEASUREMENT_LINE_1:
    case ModuleCommands::START_MEASUREMENT_LINE_2:
    //case ModuleCommands::GET_DS1820_ADDR_LINE_1:
    //case ModuleCommands::GET_DS1820_ADDR_LINE_2:
    case ModuleCommands::SET_TECH_INTERFACE:
    //case ModuleCommands::GET_TECH_INTERFACE:
    case ModuleCommands::RECV_DATA_SSI:
    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
    //case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
    //case ModuleCommands::GET_DEVICE_CLASS:
    //case ModuleCommands::GET_NOMINAL_CURRENT:
    //case ModuleCommands::GET_NOMINAL_VOLTAGE:
    //case ModuleCommands::GET_NOMINAL_POWER:
    //case ModuleCommands::GET_OVP_THRESHOLD:
    //case ModuleCommands::GET_OCP_THRESHOLD:
    case ModuleCommands::SET_OVP_THRESHOLD:
    case ModuleCommands::SET_OCP_THRESHOLD:
    case ModuleCommands::SET_SET_VALUE_U:
    case ModuleCommands::SET_SET_VALUE_I:
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
    case ModuleCommands::PSC_TRACKING_ON:
    case ModuleCommands::PSC_TRACKING_OFF:
    case ModuleCommands::SEND_TEST_ARRAY:
    //case ModuleCommands::RECEIVE_TEST_ARRAY:
    case ModuleCommands::SEND_COMMAND_ARRAY:
    //case ModuleCommands::RECEIVE_COMMAND_ARRAY:
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
    //case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
    //case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
    case ModuleCommands::START_MKO:
    case ModuleCommands::STOP_MKO:
    //case ModuleCommands::GET_MODULE_STATUS:
    case ModuleCommands::SET_MODULE_LOGIC_STATUS:
        return true;

    default:
        break;
    }

    return false;
}

QSet<uint32_t> SystemState::inputParams(int module, int command) const
{
    if (module < 0 || module >= ModuleCommands::MODULES_COUNT)
    {
        return QSet<uint32_t>();
    }

    auto it = mCommands[module].find(command);
    if (it == mCommands[module].end())
    {
        return QSet<uint32_t>();
    }

    return it.value().inputParams;
}

QSet<uint32_t> SystemState::outputParams(int module, int command) const
{
    if (module < 0 || module >= ModuleCommands::MODULES_COUNT)
    {
        return QSet<uint32_t>();
    }

    auto it = mCommands[module].find(command);
    if (it == mCommands[module].end())
    {
        return QSet<uint32_t>();
    }

    return it.value().outputParams;
}

void SystemState::updateParams()
{
    mParams.clear();

    addParam(VOLTAGE,               tr("Voltage, V"),               "U",        tr("Voltage, V"));
    addParam(CURRENT,               tr("Current, A"),               "I",        tr("Current, A"));
    addParam(TEMPERATURE,           tr("Temperature, °C"),          "T",        tr("Temperature, °C"));
    addParam(DRIVE_MODE,            tr("Mode"),                     "M",        tr("Mode"));
    addParam(STEPS,                 tr("Steps"),                    "St",       tr("Steps"));
    addParam(VELOCITY,              tr("Velocity"),                 "V",        tr("Velocity"));
    addParam(MODE_PSY,              tr("Mode (ψ)"),                 "PsyM",     tr("Mode (ψ)"));
    addParam(STEPS_PSY,             tr("Steps (ψ)"),                "PsyS",     tr("Steps (ψ)"));
    addParam(VELOCITY_PSY,          tr("Velocity (ψ)"),             "PsyV",     tr("Velocity (ψ)"));
    addParam(CURRENT_PSY,           tr("Max current (ψ)"),          "PsyI",     tr("Max current (ψ)"));
    addParam(ANGLE_PSY,             tr("Angle (ψ)"),                "PsyA",     tr("Angle (ψ)"));
    addParam(MODE_NU,               tr("Mode (υ)"),                 "NuM",      tr("Mode (υ)"));
    addParam(STEPS_NU,              tr("Steps (υ)"),                "NuS",      tr("Steps (υ)"));
    addParam(VELOCITY_NU,           tr("Velocity (υ)"),             "NuV",      tr("Velocity (υ)"));
    addParam(CURRENT_NU,            tr("Max current (υ)"),          "NuI",      tr("Max current (υ)"));
    addParam(ANGLE_NU,              tr("Angle (υ)"),                "NuA",      tr("Angle (υ)"));
    addParam(SENSOR_FLAG,           tr("Temp.Sensor Flag"),         "TSF",      tr("Temperature sensor presense flag"));
    addParam(DEVICE_CLASS,          tr("Device class"),             "Class",    tr("Device Class"));
    addParam(POWER,                 tr("Power, W"),                 "Pow",      tr("Power, W"));
    addParam(STATUS_PHYSICAL,       tr("Phys.status"),              "PActive",  tr("Pysical module status. 1 - active, 0 - inactive"));
    addParam(STATUS_LOGICAL,        tr("Logic.status"),             "LActive",  tr("Logical module status. 1 - enabled, 0 - disabled"));
    addParam(SUBADDRESS,            tr("Subaddress"),               "SubAddr",  tr("Subaddress")); // implicit param
    addParam(POWER_STATE,           tr("Power State"),              "IsOn",     tr("Power state")); // implicit param
    addParam(CHANNEL_ID,            tr("Channel"),                  "Ch",       tr("Telemetry channel index"));
    addParam(RELAY_STATE,           tr("Relay state"),              "IsOn",     tr("Relay state. 1 - on, 0 - off"));
    addParam(MODULE_ADDRESS,        tr("Module address"),           "Addr",     tr("Module address"));
    addParam(MODULE_READY,          tr("Module ready flag"),        "IsReady",  tr("Module ready flag. 1 - ready, 0 - not ready"));
    addParam(MODULE_AFTER_RESET,    tr("Module after reset flag"),  "IsReset",  tr("Module is after reset flag. 1 - after reset, 0 - not after reset"));
    addParam(MODULE_HAS_ERRORS,     tr("Module has errors flag"),   "HasErr",   tr("Module error flag. 1 - module has errors, 0 - module has no errors"));
    addParam(FUSE_ID,               tr("Fuse id"),                  "Fuse",     tr("Fuse index"));
    addParam(FUSE_STATE,            tr("Fuse state"),               "FuSt",     tr("Fuse state: 0 - fuse OK, 1 - fuse malfunction"));
    addParam(SENSOR_NUMBER,         tr("Sensor number"),            "SN",       tr("DS1820 sensor number in the line"));
    addParam(SENSORS_COUNT,         tr("Sensors count"),            "SCnt",     tr("DS1820 sensors count at the line"));
}

void SystemState::addParam(uint32_t id, const QString& name, const QString& variable, const QString& description)
{
    Param param;
    param.name = name;
    param.variable = variable;
    param.description = description;

    mParams[id] = param;
}
