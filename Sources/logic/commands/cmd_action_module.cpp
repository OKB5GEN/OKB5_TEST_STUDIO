#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"
//#include "Headers/logic/variable_controller.h"

#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

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

void CmdActionModule::setParams(ModuleCommands::ModuleID module, ModuleCommands::CommandID operation, const QMap<QString, QString>& in, const QMap<QString, QString>& out)
{
    mModule = module;
    mOperation = operation;
    mInputParams = in;
    mOutputParams = out;
    updateText();
}

ModuleCommands::CommandID CmdActionModule::operation() const
{
    return mOperation;
}

ModuleCommands::ModuleID CmdActionModule::module() const
{
    return mModule;
}

const QMap<QString, QString>& CmdActionModule::inputParams() const
{
    return mInputParams;
}

const QMap<QString, QString>& CmdActionModule::outputParams() const
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

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().isEmpty())
        {
            isValid = false;
            break;
        }
        else
        {
            if (!mText.isEmpty())
            {
                mText += ",";
            }

            mText += it.value();
        }
    }

    if (!mOutputParams.isEmpty())
    {
        mText += "=";
    }

    if (mInputParams.size() < mSystemState->paramsCount(mModule, mOperation, true))
    {
        isValid = false;
    }

    if (mOutputParams.size() < mSystemState->paramsCount(mModule, mOperation, false))
    {
        isValid = false;
    }

    mText += moduleName();
    mText += ".";
    mText += commandName();
    mText += "(";

    bool isFirstParam = true;

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().isEmpty())
        {
            isValid = false;
        }
        else
        {
            if (!isFirstParam)
            {
                mText += ",";
            }

            mText += it.value();
        }

        isFirstParam = false;
    }

    mText += ")";

    if ((hasError() && isValid) || (!hasError() && !isValid))
    {
        if (!isValid)
        {
            mText = tr("Invalid cmd");
        }

        setErrorStatus(!isValid);
    }

    emit textChanged(mText);
}

void CmdActionModule::onNameChanged(const QString& newName, const QString& oldName)
{
    QList<QString> values = mInputParams.values();

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value() == oldName)
        {
            mInputParams[it.key()] = newName;
        }
    }

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value() == oldName)
        {
            mOutputParams[it.key()] = newName;
        }
    }

    updateText();
}

void CmdActionModule::onVariableRemoved(const QString& name)
{
    QList<QString> values = mInputParams.values();

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value() == name)
        {
            mInputParams[it.key()] = "";
        }
    }

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value() == name)
        {
            mOutputParams[it.key()] = "";
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
    QString text = tr("ЗАГЛУШКА");

    switch (mOperation)
    {
    // own modules commands
    case ModuleCommands::GET_MODULE_ADDRESS:            text = tr("ПА"); break;  // Запрос адреса модуля
    case ModuleCommands::GET_STATUS_WORD:               text = tr("ПСС"); break; // Запрос статусного слова
    case ModuleCommands::RESET_ERROR:                   text = tr("СО"); break;  // Команда сброса ошибок
    case ModuleCommands::SOFT_RESET:                    text = tr("СР"); break;  // Soft reset
    case ModuleCommands::GET_SOWFTWARE_VER:             text = tr("ПВ"); break;  // Запрос версии прошивки модуля
    case ModuleCommands::ECHO:                          text = tr("ЭХО"); break; // Echo
    case ModuleCommands::POWER_CHANNEL_CTRL:            text = tr("УР"); break;  // Управление подачей питания 27В на модуле питания
    case ModuleCommands::GET_PWR_MODULE_FUSE_STATE:     text = tr("СОСТ"); break;// Запрос проверки целостности предохранителя с модулей питания
    case ModuleCommands::GET_CHANNEL_TELEMETRY:         text = tr("ТМ"); break;  // Запрос данных сигналов телеметрии СТМ
    case ModuleCommands::SET_MKO_PWR_CHANNEL_STATE:     text = tr("УП"); break;  // Команда управления питанием МКО интерфейсов

    //TODO
    // Команды CAN/RS/SSI, вероятно, будут доступны по усеченному интерфейсу типа
    // - "отправить/принять данные по И"
    // - "очистить буфер И"

    //case ModuleCommands::SET_PACKET_SIZE_CAN:           text = tr(""); break;
    //case ModuleCommands::ADD_BYTES_CAN:                 text = tr(""); break;
    //case ModuleCommands::SEND_PACKET_CAN:               text = tr(""); break;
    //case ModuleCommands::CHECK_RECV_DATA_CAN:           text = tr(""); break;
    //case ModuleCommands::RECV_DATA_CAN:                 text = tr(""); break;
    //case ModuleCommands::CLEAN_BUFFER_CAN:              text = tr(""); break;

    //case ModuleCommands::SET_PACKET_SIZE_RS485:         text = tr(""); break;
    //case ModuleCommands::ADD_BYTES_RS485:               text = tr(""); break;
    //case ModuleCommands::SEND_PACKET_RS485:             text = tr(""); break;
    //case ModuleCommands::CHECK_RECV_DATA_RS485:         text = tr(""); break;
    //case ModuleCommands::RECV_DATA_RS485:               text = tr(""); break;
    //case ModuleCommands::CLEAN_BUFFER_RS485:            text = tr(""); break;

    //case ModuleCommands::SET_MODE_RS485:                text = tr(""); break;
    //case ModuleCommands::SET_SPEED_RS485:               text = tr(""); break;
    //case ModuleCommands::SET_TECH_INTERFACE:            text = tr(""); break;
    //case ModuleCommands::GET_TECH_INTERFACE:            text = tr(""); break;
    //case ModuleCommands::RECV_DATA_SSI:                 text = tr(""); break;

    case ModuleCommands::GET_TEMPERATURE_PT100:         text = tr("ТПТ"); break;  // Запрос значений температурных датчиков ПТ-100
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:       text = tr("КDS1"); break; // Запрос количества датчиков DS1820 на линии 1
    case ModuleCommands::GET_DS1820_COUNT_LINE_2:       text = tr("КDS2"); break; // Запрос количества датчиков DS1820 на линии 2
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1: text = tr("ТDS1"); break; // Запрос значений температурных датчиков DS1820 на линии 1
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2: text = tr("ТDS2"); break; // Запрос значений температурных датчиков DS1820 на линии 2
    case ModuleCommands::GET_POWER_MODULE_STATE:        text = tr("СОСТ"); break; // Запрос состояния модуля подачи питания 27В
    case ModuleCommands::GET_MKO_MODULE_STATE:          text = tr("СОСТ"); break; // Запрос состояния МКО
    case ModuleCommands::RESET_LINE_1:                  text = tr("СDS1"); break; // Ресет датчиков на линии 1
    case ModuleCommands::RESET_LINE_2:                  text = tr("СDS2"); break; // Ресет датчиков на линии 2
    case ModuleCommands::START_MEASUREMENT_LINE_1:      text = tr("ИDS1"); break; // Запуск измерений температуры на линии 1
    case ModuleCommands::START_MEASUREMENT_LINE_2:      text = tr("ИDS2"); break; // Запуск измерений температуры на линии 2
    case ModuleCommands::GET_DS1820_ADDR_LINE_1:        text = tr("АDS1"); break; // Чтение адресов датчиков DS1820 на линии 1
    case ModuleCommands::GET_DS1820_ADDR_LINE_2:        text = tr("АDS2"); break; // Чтение адресов датчиков DS1820 на линии 2

    // Third party modules commands (arbitrary) >>>>

    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:       text = tr("УНТ"); break;
    case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:   text = tr("УО"); break;
    case ModuleCommands::SET_POWER_STATE:               text = tr("УП"); break;
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:       text = tr("ПНТ"); break;
    default:
        break;
    }

    return text;
}

void CmdActionModule::writeCustomAttributes(QXmlStreamWriter* writer)
{
    QMetaEnum module = QMetaEnum::fromType<ModuleCommands::ModuleID>();
    QMetaEnum command = QMetaEnum::fromType<ModuleCommands::CommandID>();

    writer->writeAttribute("module", module.valueToKey(mModule));
    writer->writeAttribute("command", command.valueToKey(mOperation));

    // input params
    writer->writeStartElement("input_params");
    for (QMap<QString, QString>::const_iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", it.key());
        writer->writeAttribute("variable", it.value());
        writer->writeEndElement();
    }

    writer->writeEndElement();

    // output params
    writer->writeStartElement("output_params");
    for (QMap<QString, QString>::const_iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        writer->writeStartElement("param");
        writer->writeAttribute("name", it.key());
        writer->writeAttribute("variable", it.value());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

void CmdActionModule::readCustomAttributes(QXmlStreamReader* reader)
{
    QMetaEnum module = QMetaEnum::fromType<ModuleCommands::ModuleID>();
    QMetaEnum command = QMetaEnum::fromType<ModuleCommands::CommandID>();

    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("module"))
    {
        QString str = attributes.value("module").toString();
        mModule = ModuleCommands::ModuleID(module.keyToValue(qPrintable(str)));
    }

    if (attributes.hasAttribute("command"))
    {
        QString str = attributes.value("command").toString();
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
                        QString variable;
                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
                        }

                        if (attributes.hasAttribute("variable"))
                        {
                            variable = attributes.value("variable").toString();
                        }

                        mInputParams[name] = variable;
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
                        QString variable;
                        if (attributes.hasAttribute("name"))
                        {
                            name = attributes.value("name").toString();
                        }

                        if (attributes.hasAttribute("variable"))
                        {
                            variable = attributes.value("variable").toString();
                        }

                        mOutputParams[name] = variable;
                    }

                    reader->readNext();
                }
            }
        }

        reader->readNext();
    }
}
