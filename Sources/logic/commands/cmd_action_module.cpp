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
    CmdAction(DRAKON::ACTION_MODULE, parent),
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
        finish(); // command succesfully executed
    }
    else
    {
        mErrorText = tr("Command '%1' execution failed").arg(mText);
        emit criticalError(this);
    }
}

void CmdActionModule::setParams(ModuleCommands::ModuleID module, uint32_t operation, const QMap<QString, QVariant>& in, const QMap<QString, QVariant>& out)
{
    mModule = module;
    mOperation = operation;
    mInputParams = in;
    mOutputParams = out;
    updateText();
}

uint32_t CmdActionModule::operation() const
{
    return mOperation;
}

ModuleCommands::ModuleID CmdActionModule::module() const
{
    return mModule;
}

const QMap<QString, QVariant>& CmdActionModule::inputParams() const
{
    return mInputParams;
}

const QMap<QString, QVariant>& CmdActionModule::outputParams() const
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

    if (mInputParams.size() < mSystemState->paramsCount(mModule, mOperation, true))
    {
        isValid = false;
    }

    if (mOutputParams.size() < mSystemState->paramsCount(mModule, mOperation, false))
    {
        isValid = false;
    }

    mText += commandName();

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

    emit textChanged(mText);
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

QString CmdActionModule::moduleName() const
{
    QString text;
    switch (mModule)
    {
    case ModuleCommands::POWER_UNIT_BUP:
        text = tr("БП1");
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        text = tr("БП2");
        break;
    case ModuleCommands::MKO:
        text = tr("МКО");
        break;
    case ModuleCommands::STM:
        text = tr("СТМ");
        break;
    case ModuleCommands::OTD:
        text = tr("ОТД");
        break;
    case ModuleCommands::TECH:
        text = tr("ТЕХ");
        break;
    default:
        break;
    }

    return text;
}

QString CmdActionModule::commandName() const
{
    QString text = moduleName();
    text += tr(".");

    switch (mOperation)
    {
    case ModuleCommands::GET_MODULE_STATUS:
        text += tr("ПолСтат");
        break;
    case ModuleCommands::SET_MODULE_LOGIC_STATUS:
        text += tr("УстСтат");
        break;
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
        {
            QString paramName1 = mSystemState->paramName(SystemState::CHANNEL_ID);
            QString paramName2 = mSystemState->paramName(SystemState::POWER_STATE);
            int channel = mInputParams.value(paramName1).toInt();
            int state = mInputParams.value(paramName2).toInt();

            if (state == ModuleCommands::POWER_ON)
            {
                text += tr("Вкл");
            }
            else
            {
                text += tr("Выкл");
            }

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += tr("БУПОсн");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += tr("БУПРез");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += tr("Нагр1");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += tr("Нагр2");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += tr("СилПит");
                break;
            default:
                break;
            }
        }
        break;

    case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
        {
            QString paramName1 = mSystemState->paramName(SystemState::CHANNEL_ID);
            QString paramName2 = mSystemState->paramName(SystemState::POWER_STATE);
            int channel = mInputParams.value(paramName1).toInt();
            int state = mInputParams.value(paramName2).toInt();

            if (state == ModuleCommands::POWER_ON)
            {
                text += tr("Вкл");
            }
            else
            {
                text += tr("Выкл");
            }

            switch (channel)
            {
            case ModuleCommands::MKO_1:
                text += tr("MKOОсн");
                break;
            case ModuleCommands::MKO_2:
                text += tr("MKOРез");
                break;
            default:
                break;
            }
        }
        break;
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        text = tr("ПолТелем");
        break;
    case ModuleCommands::GET_TEMPERATURE_PT100:
        text = tr("ПолТемпПТ");
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        text = tr("ПолТемпDS1");
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        text = tr("ПолТемпDS2");
        break;
    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        text += tr("УстНапр");
        break;
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        text += tr("ПолНапрТок");
        break;
    case ModuleCommands::SEND_TEST_ARRAY:
        text += tr("ПТМ");
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY:
        text += tr("ВТМ");
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY:
        text += tr("ПКМ");
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY:
        text += tr("ВКМ");
        break;
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
        text += tr("ПТМК");
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        text += tr("ВТМК");
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
        text += tr("ПКМК");
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        text += tr("ВКМК");
        break;
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
        text += tr("ВклПитДУ");
        break;
    case ModuleCommands::START_MKO:
        text += tr("Старт");
        break;
    case ModuleCommands::STOP_MKO:
        text += tr("Стоп");
        break;
        // new power module commands
    case ModuleCommands::GET_DEVICE_CLASS:
        text += tr("ПолКласс");
        break;
    case ModuleCommands::GET_NOMINAL_CURRENT:
        text += tr("ПолНомТ");
        break;
    case ModuleCommands::GET_NOMINAL_VOLTAGE:
        text += tr("ПолНомН");
        break;
    case ModuleCommands::GET_NOMINAL_POWER:
        text += tr("ПолНомМ");
        break;
    case ModuleCommands::GET_OVP_THRESHOLD:
        text += tr("ПолНОтс");
        break;
    case ModuleCommands::GET_OCP_THRESHOLD:
        text += tr("ПолТОтс");
        break;
    case ModuleCommands::SET_OVP_THRESHOLD:
        text += tr("УстНОтс");
        break;
    case ModuleCommands::SET_OCP_THRESHOLD:
        text += tr("УстТОтс");
        break;
    case ModuleCommands::SET_SET_VALUE_U:
        text += tr("УстНапр");
        break;
    case ModuleCommands::SET_SET_VALUE_I:
        text += tr("УстТок");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
        text += tr("ВклПит");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
        text += tr("ВыклПит");
        break;
    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
        text += tr("СбрОш");
        break;
    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
        text += tr("ВклУд");
        break;
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
        text += tr("ВклЛок");
        break;
    case ModuleCommands::PSC_TRACKING_ON:
        text += tr("ВклТр");
        break;
    case ModuleCommands::PSC_TRACKING_OFF:
        text += tr("ВыклТр");
        break;
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        {
            QString paramName = mSystemState->paramName(SystemState::CHANNEL_ID);
            int channel = mInputParams.value(paramName).toInt();

            text += tr("ПС");

            switch (channel)
            {
            case ModuleCommands::MKO_1:
                text += tr("МКООсн");
                break;
            case ModuleCommands::MKO_2:
                text += tr("МКОРез");
                break;
            default:
                text += tr("UNKNOWN");
                break;
            }
        }
        break;
    case ModuleCommands::GET_POWER_CHANNEL_STATE:
        {
            QString paramName = mSystemState->paramName(SystemState::CHANNEL_ID);
            int channel = mInputParams.value(paramName).toInt();

            text += tr("ПС");

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += tr("БУПОсн");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += tr("БУПРез");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += tr("Нагр1");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += tr("Нагр2");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += tr("СилПит");
                break;
            default:
                text += tr("UNKNOWN");
                break;
            }
        }
        break;
    default:
        {
            QMetaEnum commands = QMetaEnum::fromType<ModuleCommands::CommandID>();
            LOG_WARNING(QString("Command '%1' GUI text description not implemented").arg(commands.valueToKey(mOperation)));
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

    // input params
    writer->writeStartElement("input_params");
    for (auto it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", it.key());
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
        writer->writeAttribute("name", it.key());
        writer->writeAttribute("type", QString::number(int(it.value().type())));
        writer->writeAttribute("value", it.value().toString());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

void CmdActionModule::readCustomAttributes(QXmlStreamReader* reader)
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
                        QString name;

                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
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
                                mInputParams[name] = attributes.value("value").toString();
                            }
                            else //if (metaType == QMetaType::Double)
                            {
                                mInputParams[name] = attributes.value("value").toDouble();
                            }
//                            else
//                            {
//                                LOG_ERROR(QString("Unexpected input param '%1' type %2").arg(name).arg(metaType));
//                                mInputParams[name] = QVariant();
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
                        QString name;

                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
                        }

                        if (attributes.hasAttribute("variable")) // for backward compatibility TODO remove
                        {
                            QString variable;
                            variable = attributes.value("variable").toString();
                            mOutputParams[name] = variable;
                        }
                        else
                        {
                            int metaType = QMetaType::UnknownType;

                            if (attributes.hasAttribute("type"))
                            {
                                metaType = attributes.value("type").toInt();
                            }

                            if (attributes.hasAttribute("value"))
                            {
                                if (metaType == QMetaType::QString)
                                {
                                    mOutputParams[name] = attributes.value("value").toString();
                                }
                                else //if (metaType == QMetaType::Double)
                                {
                                    mOutputParams[name] = attributes.value("value").toDouble();
                                }
//                                else
//                                {
//                                    LOG_ERROR(QString("Unexpected output param '%1' type %2").arg(name).arg(metaType));
//                                    mOutputParams[name] = QVariant();
//                                }
                            }
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

