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
    ModuleCommands::ModuleID moduleBefore = mModule;
    uint32_t operationBefore = mOperation;
    QMap<QString, QVariant> inBefore = mInputParams;
    QMap<QString, QVariant> outBefore = mOutputParams;

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

    mText += moduleNameImpl();
    mText += ".";
    mText += commandName(mOperation, mInputParams);

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
            QString paramName = mSystemState->paramName(SystemState::CHANNEL_ID);
            int channel = mInputParams.value(paramName).toInt();

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
            QString paramName = mSystemState->paramName(SystemState::FUSE_ID);
            int fuse = mInputParams.value(paramName).toInt();

            if (fuse >= 1 && fuse <= 4)
            {
                text += CmdActionModule::moduleName(ModuleCommands::POWER_UNIT_BUP, false);
            }
            else if (fuse > 4 && fuse <= 8)
            {
                text += CmdActionModule::moduleName(ModuleCommands::POWER_UNIT_PNA, false);
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
        text = isFullName ? tr("Модуль питания блока управления приводом") : tr("БП1");
        break;
    case ModuleCommands::POWER_UNIT_PNA:
        text = isFullName ? tr("Модуль питания привода направленной антенны") : tr("БП2");
        break;
    case ModuleCommands::MKO:
        text = isFullName ? tr("Модуль мультиплексного канала обмена") : tr("МКО");
        break;
    case ModuleCommands::STM:
        text = isFullName ? tr("Модуль сигнальной телеметрии") : tr("СТМ");
        break;
    case ModuleCommands::OTD:
        text = isFullName ? tr("Модуль опроса температурных датчиков") : tr("ОТД");
        break;
    case ModuleCommands::DRIVE_SIMULATOR:
        text = isFullName ? tr("Модуль имитатора привода") : tr("ИП");
        break;
    case ModuleCommands::TECH:
        text = isFullName ? tr("Модуль технологический") : tr("ТЕХ");
        break;
    default:
        break;
    }

    return text;
}

QString CmdActionModule::commandFullName(uint32_t commandID) const
{
    return commandName(commandID, QMap<QString, QVariant>(), true);
}

QString CmdActionModule::commandName(uint32_t commandID, const QMap<QString, QVariant>& inputParams, bool isFullName /* = false*/) const
{
    QString text;

    switch (commandID)
    {
    case ModuleCommands::GET_MODULE_STATUS:
        text += isFullName ? tr("ЗАПРОС: статус") : tr("ПолСтат");
        break;
    case ModuleCommands::SET_MODULE_LOGIC_STATUS:
        text += isFullName ? tr("УСТАНОВКА: логический статус") : tr("УстСтат");
        break;
    case ModuleCommands::SET_POWER_CHANNEL_STATE:
        {
            QString paramName1 = mSystemState->paramName(SystemState::CHANNEL_ID);
            QString paramName2 = mSystemState->paramName(SystemState::POWER_STATE);
            int channel = inputParams.value(paramName1).toInt();
            int state = inputParams.value(paramName2).toInt();

            if (state == ModuleCommands::POWER_ON)
            {
                text += isFullName ? tr("ВКЛ: ") : tr("Вкл");
            }
            else
            {
                text += isFullName ? tr("ВЫКЛ: ") : tr("Выкл");
            }

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += isFullName ? tr("основной комплект БУП") : tr("БУПОсн");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += isFullName ? tr("резервный комплект БУП") : tr("БУПРез");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += isFullName ? tr("нагреватели ПНА на линии 1") : tr("Нагр1");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += isFullName ? tr("нагреватели ПНА на линии 2") : tr("Нагр2");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += isFullName ? tr("подачу силового питания") : tr("СилПит");
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
            int channel = inputParams.value(paramName1).toInt();
            int state = inputParams.value(paramName2).toInt();

            if (state == ModuleCommands::POWER_ON)
            {
                text += isFullName ? tr("ВКЛ: ") : tr("Вкл");
            }
            else
            {
                text += isFullName ? tr("ВЫКЛ: ") : tr("Выкл");
            }

            switch (channel)
            {
            case ModuleCommands::MKO_1:
                text += isFullName ? tr("подачу питания на МКО Осн.") : tr("MKOОсн");
                break;
            case ModuleCommands::MKO_2:
                text += isFullName ? tr("подачу питания на МКО Рез.") : tr("MKOРез");
                break;
            default:
                break;
            }
        }
        break;
    case ModuleCommands::GET_FUSE_STATE:
        text += isFullName ? tr("ЗАПРОС: состояние предохранителя") : tr("ПолСостПр");
        break;
    case ModuleCommands::GET_CHANNEL_TELEMETRY:
        text += isFullName ? tr("ЗАПРОС: телеметрия канала") : tr("ПолТелем");
        break;
    case ModuleCommands::GET_TEMPERATURE_PT100:
        {
            QString paramName = mSystemState->paramName(SystemState::SENSOR_NUMBER);
            int sensorID = inputParams.value(paramName).toInt();
            if (isFullName)
            {
                text += tr("ЗАПРОС: температура с датчика ПТ-100 #%1").arg(sensorID);
            }
            else
            {
                text += tr("ПолТемпПТ%1").arg(sensorID);
            }
        }
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        text += isFullName ? tr("ЗАПРОС: температура с датчика DS1820 линия 1") : tr("ПолТемпDS1");
        break;
    case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        text += isFullName ? tr("ЗАПРОС: температура с датчика DS1820 линия 2") : tr("ПолТемпDS2");
        break;
    case ModuleCommands::GET_DS1820_COUNT_LINE_1:
        text += isFullName ? tr("ЗАПРОС: количество датчиков DS1820 на линии 1") : tr("ПолЧислоDS1");
        break;
    case ModuleCommands::GET_DS1820_COUNT_LINE_2:
        text += isFullName ? tr("ЗАПРОС: количество датчиков DS1820 на линии 2") : tr("ПолЧислоDS2");
        break;
    case ModuleCommands::START_MEASUREMENT_LINE_1:
        text += isFullName ? tr("ЗАПРОС: запуск измерений температуры датчиками DS1820 на линии 1") : tr("ЗапускИзм1");
        break;
    case ModuleCommands::START_MEASUREMENT_LINE_2:
        text += isFullName ? tr("ЗАПРОС: запуск измерений температуры датчиками DS1820 на линии 2") : tr("ЗапускИзм2");
        break;
    case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
        text += isFullName ? tr("УСТАНОВКА: выходное напряжение (максимальная мощность)") : tr("УстНапр");
        break;
    case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
        text += isFullName ? tr("ЗАПРОС: текущий ток и напряжение") : tr("ПолНапрТок");
        break;
    case ModuleCommands::SEND_TEST_ARRAY:
        text += isFullName ? tr("ОТПРАВКА: тестовый массив") : tr("ПТМ");
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY:
        text += isFullName ? tr("ЗАПРОС: тестовый массив") : tr("ВТМ");
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY:
        text += isFullName ? tr("ОТПРАВКА: командный массив") : tr("ПКМ");
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY:
        text += isFullName ? tr("ЗАПРОС: командный массив") : tr("ВКМ");
        break;
    case ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL:
        {
            text += isFullName ? tr("ОТПРАВКА: тестовый массив по линии ") : tr("ПТМК");

            if (isFullName)
            {
                QString paramName = mSystemState->paramName(SystemState::SUBADDRESS);
                int channel = inputParams.value(paramName).toInt();
                if (channel == ModuleMKO::PSY_CHANNEL_SUBADDRESS)
                {
                    text += QString("ψ");
                }
                else
                {
                    text += QString("υ");
                }
            }

            text += tr("");
        }
        break;
    case ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL:
        {
            text += isFullName ? tr("ЗАПРОС: тестовый массив по линии ") : tr("ВТМК");

            if (isFullName)
            {
                QString paramName = mSystemState->paramName(SystemState::SUBADDRESS);
                int channel = inputParams.value(paramName).toInt();
                if (channel == ModuleMKO::PSY_CHANNEL_SUBADDRESS)
                {
                    text += QString("ψ");
                }
                else
                {
                    text += QString("υ");
                }
            }
        }
        break;
    case ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL:
        {
            text += isFullName ? tr("ОТПРАВКА: командный массив по линии ") : tr("ПКМК");

            if (isFullName)
            {
                QString paramName = mSystemState->paramName(SystemState::SUBADDRESS);
                int channel = inputParams.value(paramName).toInt();
                if (channel == ModuleMKO::PSY_CHANNEL_SUBADDRESS)
                {
                    text += QString("ψ");
                }
                else
                {
                    text += QString("υ");
                }
            }
        }
        break;
    case ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL:
        {
            text += isFullName ? tr("ЗАПРОС: командный массив по линии ") : tr("ВКМК");

            if (isFullName)
            {
                QString paramName = mSystemState->paramName(SystemState::SUBADDRESS);
                int channel = inputParams.value(paramName).toInt();
                if (channel == ModuleMKO::PSY_CHANNEL_SUBADDRESS)
                {
                    text += QString("ψ");
                }
                else
                {
                    text += QString("υ");
                }
            }
        }
        break;
    case ModuleCommands::SEND_TO_ANGLE_SENSOR:
        {
            QString paramName = mSystemState->paramName(SystemState::SUBADDRESS);
            int source = inputParams.value(paramName).toInt();

            if (source == ModuleMKO::PS_FROM_MAIN_KIT)
            {
                text += isFullName ? tr("ОТПРАВКА: подача питания на ДУ (осн. комплект)") : tr("ПитДУОсн");
            }
            else if (source == ModuleMKO::PS_FROM_RESERVE_KIT)
            {
                text += isFullName ? tr("ОТПРАВКА: подача питания на ДУ (рез. комплект)") : tr("ПитДУРез");
            }
        }
        break;
    case ModuleCommands::START_MKO:
        text += isFullName ? tr("СТАРТ модуля") : tr("Старт");
        break;
    case ModuleCommands::STOP_MKO:
        text += isFullName ? tr("СТОП модуля") : tr("Стоп");
        break;
    case ModuleCommands::GET_DEVICE_CLASS:
        text += isFullName ? tr("ЗАПРОС: класс устройства") : tr("ПолКласс");
        break;
    case ModuleCommands::GET_NOMINAL_CURRENT:
        text += isFullName ? tr("ЗАПРОС: номинальный ток") : tr("ПолНомТ");
        break;
    case ModuleCommands::GET_NOMINAL_VOLTAGE:
        text += isFullName ? tr("ЗАПРОС: номинальное напряжение") : tr("ПолНомН");
        break;
    case ModuleCommands::GET_NOMINAL_POWER:
        text += isFullName ? tr("ЗАПРОС: номинальную мощность") : tr("ПолНомМ");
        break;
    case ModuleCommands::GET_OVP_THRESHOLD:
        text += isFullName ? tr("ЗАПРОС: напряжение отсечки") : tr("ПолНОтс");
        break;
    case ModuleCommands::GET_OCP_THRESHOLD:
        text += isFullName ? tr("ЗАПРОС: ток отсечки") : tr("ПолТОтс");
        break;
    case ModuleCommands::SET_OVP_THRESHOLD:
        text += isFullName ? tr("УСТАНОВКА: напряжение отсечки") : tr("УстНОтс");
        break;
    case ModuleCommands::SET_OCP_THRESHOLD:
        text += isFullName ? tr("УСТАНОВКА: ток отсечки") : tr("УстТОтс");
        break;
    case ModuleCommands::SET_SET_VALUE_U:
        text += isFullName ? tr("УСТАНОВКА: выходное напряжение") : tr("УстНапр");
        break;
    case ModuleCommands::SET_SET_VALUE_I:
        text += isFullName ? tr("УСТАНОВКА: выходной ток") : tr("УстТок");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON:
        text += isFullName ? tr("ВКЛ: подачу питания на выход") : tr("ВклПит");
        break;
    case ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF:
        text += isFullName ? tr("ВЫКЛ: подачу питания на выход") : tr("ВыклПит");
        break;
    case ModuleCommands::PSC_ACKNOWLEDGE_ALARMS:
        text += isFullName ? tr("СБРОС: ошибки") : tr("СбрОш");
        break;
    case ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL:
        text += isFullName ? tr("ВКЛ: режим удаленного управления") : tr("ВклУд");
        break;
    case ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL:
        text += isFullName ? tr("ВКЛ: режим ручного управления") : tr("ВклЛок");
        break;
    case ModuleCommands::PSC_TRACKING_ON:
        text += isFullName ? tr("ВКЛ: трекинг") : tr("ВклТр");
        break;
    case ModuleCommands::PSC_TRACKING_OFF:
        text += isFullName ? tr("ВЫКЛ: трекинг") : tr("ВыклТр");
        break;
    case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        {
            QString paramName = mSystemState->paramName(SystemState::CHANNEL_ID);
            int channel = inputParams.value(paramName).toInt();

            if (!isFullName)
            {
                text += tr("ПС");
            }

            switch (channel)
            {
            case ModuleCommands::MKO_1:
                text += isFullName ? tr("ЗАПРОС: состояние питания основного комплекта МКО") : tr("МКООсн");
                break;
            case ModuleCommands::MKO_2:
                text += isFullName ? tr("ЗАПРОС: состояние питания резервного комплекта МКО") : tr("МКОРез");
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
            int channel = inputParams.value(paramName).toInt();

            if (!isFullName)
            {
                text += tr("ПС");
            }

            switch (channel)
            {
            case ModuleCommands::BUP_MAIN:
                text += isFullName ? tr("ЗАПРОС: состояние питания основного комплекта БУП") : tr("БУПОсн");
                break;
            case ModuleCommands::BUP_RESERVE:
                text += isFullName ? tr("ЗАПРОС: состояние питания резервного комплекта БУП") : tr("БУПРез");
                break;
            case ModuleCommands::HEATER_LINE_1:
                text += isFullName ? tr("ЗАПРОС: состояние нагревателей ПНА на линии 1") : tr("Нагр1");
                break;
            case ModuleCommands::HEATER_LINE_2:
                text += isFullName ? tr("ЗАПРОС: состояние нагревателей ПНА на линии 2") : tr("Нагр2");
                break;
            case ModuleCommands::DRIVE_CONTROL:
                text += isFullName ? tr("ЗАПРОС: состояние силового питания") : tr("СилПит");
                break;
            default:
                text += tr("UNKNOWN");
                break;
            }
        }
        break;
    case ModuleCommands::GET_MODULE_ADDRESS:
        {
            QString paramName = mSystemState->paramName(SystemState::MODULE_ADDRESS);
            int address = inputParams.value(paramName).toInt();

            if (!isFullName)
            {
                text += tr("ПолАдр");
            }

            switch (address)
            {
            case ModuleCommands::DEFAULT:
                text += isFullName ? tr("ЗАПРОС: адрес модуля по умолчанию") : tr("У");
                break;
            case ModuleCommands::CURRENT:
                text += isFullName ? tr("ЗАПРОС: текущий адрес модуля") : tr("Т");
                break;
            default:
                text += tr("UNKNOWN");
                break;
            }
        }
        break;
    case ModuleCommands::RESET_LINE_1:
        text += isFullName ? tr("СБРОС: датчиков на линии 1") : tr("СбрЛин1");
        break;
    case ModuleCommands::RESET_LINE_2:
        text += isFullName ? tr("СБРОС: датчиков на линии 2"): tr("СбрЛин2");
        break;
    case ModuleCommands::GET_STATUS_WORD:
        text += isFullName ? tr("ЗАПРОС: статусное слово") : tr("ПолСС");
        break;
    case ModuleCommands::RESET_ERROR:
        text += isFullName ? tr("СБРОС: ошибки") : tr("СбрОш");
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
