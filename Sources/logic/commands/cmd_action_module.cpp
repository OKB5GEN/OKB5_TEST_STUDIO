#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"
#include "Headers/logger/Logger.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>
#include <QVariant>

CmdActionModule::CmdActionModule(QObject* parent):
    Command(DRAKON::ACTION_MODULE, 1, parent),
    mModule(ModuleCommands::POWER_UNIT_BUP),
    mOperation(ModuleCommands::SET_VOLTAGE_AND_CURRENT)
{
    updateText();
}

void CmdActionModule::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(execute()));
    }
    else
    {
        execute();
    }
}

void CmdActionModule::execute()
{
    connect(mSystemState, SIGNAL(commandFinished(bool)), this, SLOT(onCommandFinished(bool)));
    mSystemState->sendCommand(this);
}

void CmdActionModule::onCommandFinished(bool success)
{
    disconnect(mSystemState, SIGNAL(commandFinished(bool)), this, SLOT(onCommandFinished(bool)));

    if (success)
    {
        emit finished(nextCommand()); // command succesfully executed
    }
    else
    {
        mErrorText = tr("Command '%1' execution failed").arg(mText);
        emit criticalError(this);
    }
}

void CmdActionModule::setParams(ModuleCommands::ModuleID module, uint32_t operation, const QMap<uint32_t, QVariant>& in, const QMap<uint32_t, QVariant>& out)
{
    ModuleCommands::ModuleID moduleBefore = mModule;
    uint32_t operationBefore = mOperation;
    QMap<uint32_t, QVariant> inBefore = mInputParams;
    QMap<uint32_t, QVariant> outBefore = mOutputParams;

    mModule = module;
    mOperation = operation;
    mInputParams = in;
    mOutputParams = out;

    bool isDataChanged = ((moduleBefore != mModule)
                          || (operationBefore != mOperation));

    if (!isDataChanged) // module and command does not changed, check command input parameters change
    {
        for (auto it = inBefore.begin(); it != inBefore.end(); ++it)
        {
            auto iter = mInputParams.find(it.key());
            if (iter != mInputParams.end())
            {
                if (iter.value() != it.value())
                {
                    isDataChanged = true;
                    break;
                }
            }
        }
    }

    if (!isDataChanged) // module and command does not changed, check command output parameters change
    {
        for (auto it = outBefore.begin(); it != outBefore.end(); ++it)
        {
            auto iter = mOutputParams.find(it.key());
            if (iter != mOutputParams.end())
            {
                if (iter.value() != it.value())
                {
                    isDataChanged = true;
                    break;
                }
            }
        }
    }

    if (isDataChanged)
    {
        updateText();
    }
}

uint32_t CmdActionModule::operation() const
{
    return mOperation;
}

ModuleCommands::ModuleID CmdActionModule::module() const
{
    return mModule;
}

const QMap<uint32_t, QVariant>& CmdActionModule::inputParams() const
{
    return mInputParams;
}

const QMap<uint32_t, QVariant>& CmdActionModule::outputParams() const
{
    return mOutputParams;
}

void CmdActionModule::updateText()
{
    if (!mSystemState)
    {
        mText = tr("Invalid cmd");
        setErrorStatus(true);
        return;
    }

    mText = "";
    bool isValid = true;

    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (!it.value().isValid())
        {
            isValid = false;
            break;
        }
    }

    if (mInputParams.size() < mSystemState->inputParams(mModule, mOperation).size())
    {
        isValid = false;
    }

    if (mOutputParams.size() < mSystemState->outputParams(mModule, mOperation).size())
    {
        isValid = false;
    }

    mText += moduleNameImpl();
    mText += ": ";
    mText += commandName(mModule, mOperation, mInputParams);
    mText += paramsText(mModule, mOperation, mInputParams);

    // TODO some hack for ON/OFF commands text
    int state = mInputParams.value(SystemState::POWER_STATE, -1).toInt();
    if (state != -1)
    {
        if (state != 0)
        {
            mText += tr(" ON");
        }
        else
        {
            mText += tr(" OFF");
        }
    }

    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (!it.value().isValid())
        {
            isValid = false;
            break;
        }
    }

    if (!isValid)
    {
        mText = tr("Invalid cmd");
    }

    if ((hasError() && isValid) || (!hasError() && !isValid))
    {
        setErrorStatus(!isValid);
    }

    emit dataChanged(mText);
}

QString CmdActionModule::moduleNameImpl() const
{
    QString text;

    switch (mOperation)
    {
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
    case ModuleCommands::GET_POWER_CHANNEL_STATE:
        {
            int channel = mInputParams.value(SystemState::CHANNEL_ID).toInt();

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
            case ModuleCommands::BUP_RESERVE:
            case ModuleCommands::RESERVED_RELAY_3:
                text += CmdActionModule::moduleName(ModuleCommands::POWER_UNIT_BUP, false);
                break;
            default:
                text += CmdActionModule::moduleName(ModuleCommands::POWER_UNIT_PNA, false);
                break;
            }
        }
        break;
    case ModuleCommands::GET_FUSE_STATE:
        {
            int fuse = mInputParams.value(SystemState::FUSE_ID).toInt();

            if (fuse >= 1 && fuse <= 8) //TODO magic numbers, [1, 4] PU1, [5, 8] PU2
            {
                text += CmdActionModule::moduleName(mModule, false);
            }
            else
            {
                LOG_ERROR(QString("Incorrect fuse id=%1 in command").arg(fuse));
            }
        }
        break;
    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        {
            text += CmdActionModule::moduleName(ModuleCommands::MKO, false);
        }
        break;

    default:
        text += moduleName(false);
        break;
    }

    return text;
}

void CmdActionModule::onNameChanged(const QString& newName, const QString& oldName)
{
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().type() == QMetaType::QString && it.value().toString() == oldName)
        {
            mInputParams[it.key()] = QVariant(newName);
        }
    }

    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().type() == QMetaType::QString && it.value().toString() == oldName)
        {
            mOutputParams[it.key()] = QVariant(newName);
        }
    }

    updateText();
}

void CmdActionModule::onVariableRemoved(const QString& name)
{
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().type() == QMetaType::QString && it.value().toString() == name)
        {
            it.value().clear();
        }
    }

    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().type() == QMetaType::QString && it.value().toString() == name)
        {
            it.value().clear();
        }
    }

    updateText();
}

QString CmdActionModule::moduleName(bool isFullName) const
{
    return moduleName(mModule, isFullName);
}

QString CmdActionModule::moduleName(int moduleId, bool isFullName)
{
    QString text;
    switch (moduleId)
    {
    case ModuleCommands::POWER_UNIT_BUP:
        text = isFullName ? tr("DCU power unit module") : tr("PU1");
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        text = isFullName ? tr("DAD power unit module") : tr("PU2");
        break;
    case ModuleCommands::MKO:
        text = isFullName ? tr("Multiplex channel module") : tr("MKO");
        break;
    case ModuleCommands::STM:
        text = isFullName ? tr("Signalling telemetry module") : tr("STM");
        break;
    case ModuleCommands::OTD:
        text = isFullName ? tr("Temperature sensor control unit") : tr("OTD");
        break;
    case ModuleCommands::DRIVE_SIMULATOR:
        text = isFullName ? tr("Drive simulator unit") : tr("DS");
        break;
    case ModuleCommands::TECH:
        text = isFullName ? tr("Technological module") : tr("Tech");
        break;
    default:
        break;
    }

    return text;
}

QString CmdActionModule::paramsText(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& inputParams)
{
    int paramID = INT_MAX;

    switch (commandID)
    {
    case ModuleCommands::GET_FUSE_STATE:
        paramID = SystemState::FUSE_ID;
        break;
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        paramID = SystemState::CHANNEL_ID;
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        paramID = SystemState::SENSOR_NUMBER;
        break;

    default:
        return QString();
        break;
    }

    QVariant value = inputParams.value(paramID);
    if (!value.isValid())
    {
        return QString();
    }

    bool ok = false;
    int valueInt = value.toInt(&ok);
    if (ok)
    {
        return QString(" #%1").arg(valueInt);
    }

    return QString(" #%1").arg(value.toString());
}

QString CmdActionModule::commandName(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& inputParams)
{
    // DCU - Drive Control Unit - БУП
    // DAD - Directional Antenna Drive - ПНА
    QString text;

    switch (commandID)
    {
    case ModuleCommands::GET_MODULE_STATUS:
        text += tr("Module status");
        break;
    case ModuleCommands::SET_MODULE_LOGIC_STATUS:
        text += tr("Logical status");
        break;
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
        {
            int channel = inputParams.value(SystemState::CHANNEL_ID).toInt();

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += tr("DCU main kit power");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += tr("DCU reserve kit power");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += tr("DAD heaters line 1 power");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += tr("DAD heaters line 2 power");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += tr("DCU power supply");
                break;
            default:
                break;
            }
        }
        break;

    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
        {
            int channel = inputParams.value(SystemState::CHANNEL_ID).toInt();

            switch (channel)
            {
            case ModuleCommands::MKO_1:
                text += tr("MKO main kit power supply");
                break;
            case ModuleCommands::MKO_2:
                text += tr("MKO reserve kit power supply");
                break;
            default:
                break;
            }
        }
        break;
    case ModuleCommands::GET_FUSE_STATE:
        text += tr("Fuse state");
        break;
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        text += tr("Channel telemetry");
        break;
    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            int sensorID = inputParams.value(SystemState::SENSOR_NUMBER).toInt();
            text += tr("Temperature PT-100 #%1").arg(sensorID);
        }
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        {
            text += tr("Temperature DS1820");
            if (moduleID == ModuleCommands::OTD)
            {
                text += tr(" (line 1)");
            }
        }
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        text += tr("Temperature DS1820 (line 2)");
        break;
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        {
            text += tr("DS1820 count");
            if (moduleID == ModuleCommands::OTD)
            {
                text += tr(" (line 1)");
            }
        }
        break;
    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
        text += tr("DS1820 count (line 2)");
        break;
    case ModuleCommands::START_MEASUREMENT_LINE_1:
        {
            text += tr("Start DS1820 measurement");
            if (moduleID == ModuleCommands::OTD)
            {
                text += tr(" (line 1)");
            }
        }
        break;
    case ModuleCommands::START_MEASUREMENT_LINE_2:
        text += tr("Start DS1820 measurement (line 2)");
        break;
    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        text += tr("Voltage (max power)");
        break;
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        text += tr("Voltage and current");
        break;
    case ModuleCommands::SEND_TEST_ARRAY:
    case ModuleCommands::RECEIVE_TEST_ARRAY:
        {
            text += tr("Test array");
        }
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY:
    case ModuleCommands::RECEIVE_COMMAND_ARRAY:
        {
            text += tr("Command array");
        }
        break;
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
    case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
    case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            QString arrayType;
            if (commandID == ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL || commandID == ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL)
            {
                arrayType = tr("Test");
            }
            else
            {
                arrayType = tr("Command");
            }

            QString line;
            int channel = inputParams.value(SystemState::SUBADDRESS).toInt();
            if (channel == ModuleMKO::PSY_CHANNEL_SUBADDRESS)
            {
                line += QString("ψ");
            }
            else
            {
                line += QString("υ");
            }

            text += tr("%1 array (line %2)").arg(arrayType).arg(line);
        }
        break;
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
        {
            int source = inputParams.value(SystemState::SUBADDRESS).toInt();
            QString kitName;
            if (source == ModuleMKO::PS_FROM_MAIN_KIT)
            {
                kitName += tr("main");
            }
            else if (source == ModuleMKO::PS_FROM_RESERVE_KIT)
            {
                kitName += tr("reserve");
            }

            text += tr("Angle sensor power supply via %1 kit").arg(kitName);
        }
        break;
    case ModuleCommands::START_MKO:
        text += tr("Enable");
        break;
    case ModuleCommands::STOP_MKO:
        text += tr("Disable");
        break;
    case ModuleCommands::GET_DEVICE_CLASS:
        text += tr("Device class");
        break;
    case ModuleCommands::GET_NOMINAL_CURRENT:
        text += tr("Max output current");
        break;
    case ModuleCommands::GET_NOMINAL_VOLTAGE:
        text += tr("Max output voltage");
        break;
    case ModuleCommands::GET_NOMINAL_POWER:
        text += tr("Max output power");
        break;
    case ModuleCommands::SET_OVP_THRESHOLD:
    case ModuleCommands::GET_OVP_THRESHOLD:
        text += tr("OVP threshold");
        break;
    case ModuleCommands::SET_OCP_THRESHOLD:
    case ModuleCommands::GET_OCP_THRESHOLD:
        text += tr("OCP threshold");
        break;
    case ModuleCommands::SET_SET_VALUE_U:
        text += tr("Voltage");
        break;
    case ModuleCommands::SET_SET_VALUE_I:
        text += tr("Current");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
        text += tr("Power output ON");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
        text += tr("Power output OFF");
        break;
    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
        text += tr("Acknowledge alarms");
        break;
    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
        text += tr("Remote control ON");
        break;
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
        text += tr("Manual control ON");
        break;
    case ModuleCommands::PSC_TRACKING_ON:
        text += tr("Tracking ON");
        break;
    case ModuleCommands::PSC_TRACKING_OFF:
        text += tr("Tracking OFF");
        break;
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        {
            int channel = inputParams.value(SystemState::CHANNEL_ID).toInt();

            QString kitName;
            switch (channel)
            {
            case ModuleCommands::MKO_1:
                kitName += tr("main");
                break;
            case ModuleCommands::MKO_2:
                kitName += tr("reserve");
                break;
            default:
                kitName += tr("UNKNOWN");
                break;
            }

            text += tr("MKO %1 kit power supply state").arg(kitName);
        }
        break;
    case ModuleCommands::GET_POWER_CHANNEL_STATE:
        {
            int channel = inputParams.value(SystemState::CHANNEL_ID).toInt();

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += tr("DCU main kit power state");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += tr("DCU reserve kit power state");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += tr("DAD heaters line 1 power state");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += tr("DAD heaters line 2 power state");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += tr("DCU power supply");
                break;
            default:
                break;
            }
        }
        break;
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            int address = inputParams.value(SystemState::MODULE_ADDRESS).toInt();

            QString addressType;
            switch (address)
            {
            case ModuleCommands::DEFAULT:
                addressType += tr("default");
                break;
            case ModuleCommands::CURRENT:
                addressType += tr("current");
                break;
            default:
                text += tr("UNKNOWN");
                break;
            }

            text += tr("Module %1 address").arg(addressType);
        }
        break;
    case ModuleCommands::RESET_LINE_1:
        {
            text += tr("DS1820 reset");
            if (moduleID == ModuleCommands::OTD)
            {
                text += tr(" (line 1)");
            }
        }
        break;
    case ModuleCommands::RESET_LINE_2:
        text += tr("DS1820 reset (line 2)");
        break;
    case ModuleCommands::GET_STATUS_WORD:
        text += tr("Status word");
        break;
    case ModuleCommands::RESET_ERROR:
        text += tr("Reset error");
        break;
    default:
        {
            text += tr("UNKNOWN");
            QMetaEnum commands = QMetaEnum::fromType<ModuleCommands::CommandID>();
            LOG_WARNING(QString("Command '%1' GUI text description not implemented").arg(commands.valueToKey(commandID)));
        }
        break;
    }

    return text;
}

void CmdActionModule::writeCustomAttributes(QXmlStreamWriter* writer)
{
    QMetaEnum module = QMetaEnum::fromType<ModuleCommands::ModuleID>();
    writer->writeAttribute("module", module.valueToKey(mModule));

    QMetaEnum command = QMetaEnum::fromType<ModuleCommands::CommandID>();
    writer->writeAttribute("command", command.valueToKey(mOperation));

    QMetaEnum params = QMetaEnum::fromType<SystemState::ParamID>();

    // input params
    writer->writeStartElement("input_params");
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", params.valueToKey(it.key()));
        writer->writeAttribute("type", QString::number(int(it.value().type())));
        writer->writeAttribute("value", it.value().toString());
        writer->writeEndElement();
    }

    writer->writeEndElement();

    // output params
    writer->writeStartElement("output_params");
    for (auto it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", params.valueToKey(it.key()));
        writer->writeAttribute("type", QString::number(int(it.value().type())));
        writer->writeAttribute("value", it.value().toString());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

void CmdActionModule::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
{
    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("module"))
    {
        QMetaEnum module = QMetaEnum::fromType<ModuleCommands::ModuleID>();
        QString str = attributes.value("module").toString();
        mModule = ModuleCommands::ModuleID(module.keyToValue(qPrintable(str)));
    }

    if (attributes.hasAttribute("command"))
    {
        QString str = attributes.value("command").toString();
        QMetaEnum command = QMetaEnum::fromType<ModuleCommands::CommandID>();
        mOperation = ModuleCommands::CommandID(command.keyToValue(qPrintable(str)));
    }

    QMetaEnum params = QMetaEnum::fromType<SystemState::ParamID>();

    while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == "command"))
    {
        if (reader->tokenType() == QXmlStreamReader::StartElement)
        {
            QString tokenName = reader->name().toString();

            if (tokenName == "input_params")
            {
                while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == tokenName))
                {
                    if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "param")
                    {
                        attributes = reader->attributes();
                        uint32_t id;

                        if (attributes.hasAttribute("name"))
                        {
                            QString str = attributes.value("name").toString();
                            id = params.keyToValue(qPrintable(str));
                        }

                        int metaType = QMetaType::UnknownType;

                        if (attributes.hasAttribute("type"))
                        {
                            metaType = attributes.value("type").toInt();
                        }

                        if (attributes.hasAttribute("value"))
                        {
                            if (metaType == QMetaType::QString)
                            {
                                mInputParams[id] = attributes.value("value").toString();
                            }
                            else //if (metaType == QMetaType::Double)
                            {
                                mInputParams[id] = attributes.value("value").toDouble();
                            }
//                            else
//                            {
//                                LOG_ERROR(QString("Unexpected input param '%1' type %2").arg(name).arg(metaType));
//                                mInputParams[id] = QVariant();
//                            }
                        }
                    }

                    reader->readNext();
                }
            }
            else if (tokenName == "output_params")
            {
                while (!(reader->tokenType() == QXmlStreamReader::EndElement && reader->name() == tokenName))
                {
                    if (reader->tokenType() == QXmlStreamReader::StartElement && reader->name() == "param")
                    {
                        attributes = reader->attributes();
                        uint32_t id;

                        if (attributes.hasAttribute("name"))
                        {
                            QString str = attributes.value("name").toString();
                            id = params.keyToValue(qPrintable(str));
                        }

                        int metaType = QMetaType::UnknownType;

                        if (attributes.hasAttribute("type"))
                        {
                            metaType = attributes.value("type").toInt();
                        }

                        if (attributes.hasAttribute("value"))
                        {
                            if (metaType == QMetaType::QString)
                            {
                                mOutputParams[id] = attributes.value("value").toString();
                            }
                            else //if (metaType == QMetaType::Double)
                            {
                                mOutputParams[id] = attributes.value("value").toDouble();
                            }
//                                else
//                                {
//                                    LOG_ERROR(QString("Unexpected output param '%1' type %2").arg(name).arg(metaType));
//                                    mOutputParams[name] = QVariant();
//                                }
                        }
                    }

                    reader->readNext();
                }
            }
        }

        reader->readNext();
    }

    updateText();
}

bool CmdActionModule::loadFromImpl(Command* other)
{
    CmdActionModule* otherModuleCmd = qobject_cast<CmdActionModule*>(other);
    if (!otherModuleCmd)
    {
        LOG_ERROR(QString("Command type mismatch (not module cmd)"));
        return false;
    }

    mModule = otherModuleCmd->module();
    mOperation = otherModuleCmd->operation();
    mInputParams = otherModuleCmd->inputParams();
    mOutputParams = otherModuleCmd->outputParams();

    return true;
}
