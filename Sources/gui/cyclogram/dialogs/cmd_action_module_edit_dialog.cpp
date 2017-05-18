#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/logger/Logger.h"
#include "Headers/gui/tools/console_text_widget.h"

#include <QtWidgets>

CmdActionModuleEditDialog::CmdActionModuleEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Module operation"));

    //adjustSize();
    //setFixedSize(sizeHint());

    setMinimumSize(QSize(1200, 500));
}

CmdActionModuleEditDialog::~CmdActionModuleEditDialog()
{

}

void CmdActionModuleEditDialog::setupUI()
{
    QGridLayout * layout = new QGridLayout(this);
    mValidator = new QDoubleValidator(this);

    mModules = new QListWidget(this);
    mModules->addItem(tr("Блок питания БУП"));
    mModules->addItem(tr("Блок питания ПНА"));
    mModules->addItem(tr("МКО"));
    mModules->addItem(tr("СТМ"));
    mModules->addItem(tr("ОТД"));
    mModules->addItem(tr("Технологический"));

    layout->addWidget(mModules, 0, 0);

    mCommands = new QListWidget(this);
    layout->addWidget(mCommands, 0, 1);

    mInParams = new QTableWidget(this);
    QStringList headers;
    headers.append(tr("Вх.Параметр"));
    headers.append(tr(""));
    headers.append(tr("Переменная"));
    headers.append(tr(""));
    headers.append(tr("Значение"));
    mInParams->setColumnCount(headers.size());
    mInParams->setHorizontalHeaderLabels(headers);
    layout->addWidget(mInParams, 0, 2);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Вых.Параметр"));
    outHeaders.append(tr("Переменная"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
    layout->addWidget(mOutParams, 0, 3);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 1, 0, 1, 4);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(mModules, SIGNAL(currentRowChanged(int)), this, SLOT(onModuleChanged(int)));
    connect(mCommands, SIGNAL(currentRowChanged(int)), this, SLOT(onCommandChanged(int)));

    setLayout(layout);
}

void CmdActionModuleEditDialog::setCommand(CmdActionModule* command)
{
    mCommand = command;

    if (mCommand)
    {
        mModules->setCurrentRow(mCommand->module());
        mConsoleTextWidget->setCommand(mCommand);
    }
}

void CmdActionModuleEditDialog::onModuleChanged(int index)
{
    mModuleID = index;
    mCommands->clear();

    SystemState* sysState = mCommand->systemState();

    // "Abstract" module commands
    addCommand(tr("ЗАПРОС: статус"), ModuleCommands::GET_MODULE_STATUS);
    addCommand(tr("УСТАНОВКА: логический статус"), ModuleCommands::SET_MODULE_LOGIC_STATUS);

    // module changed -> update command list for this module
    switch (index)
    {
    case ModuleCommands::POWER_UNIT_BUP:
    case ModuleCommands::POWER_UNIT_PNA:
        {
            // "getters" (just gathering current device state)
            addCommand(tr("ЗАПРОС: текущий ток и напряжение"), ModuleCommands::GET_VOLTAGE_AND_CURRENT);
            addCommand(tr("ЗАПРОС: класс устройства"), ModuleCommands::GET_DEVICE_CLASS);
            addCommand(tr("ЗАПРОС: номинальное напряжение"), ModuleCommands::GET_NOMINAL_VOLTAGE);
            addCommand(tr("ЗАПРОС: номинальный ток"), ModuleCommands::GET_NOMINAL_CURRENT);
            addCommand(tr("ЗАПРОС: номинальную мощность"), ModuleCommands::GET_NOMINAL_POWER);
            addCommand(tr("ЗАПРОС: напряжение отсечки"), ModuleCommands::GET_OVP_THRESHOLD);
            addCommand(tr("ЗАПРОС: ток отсечки"), ModuleCommands::GET_OCP_THRESHOLD);

            // "setters" (changing current device state)
            addCommand(tr("УСТАНОВКА: выходное напряжение (максимальная мощность)"), ModuleCommands::SET_VOLTAGE_AND_CURRENT);
            addCommand(tr("УСТАНОВКА: напряжение отсечки"), ModuleCommands::SET_OVP_THRESHOLD);
            addCommand(tr("УСТАНОВКА: ток отсечки"), ModuleCommands::SET_OCP_THRESHOLD);
            addCommand(tr("СБРОС: ошибки"), ModuleCommands::PSC_ACKNOWLEDGE_ALARMS);
            addCommand(tr("ВКЛ: режим удаленного управления"), ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL);
            addCommand(tr("ВКЛ: режим ручного управления"), ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL);
            addCommand(tr("ВКЛ: подачу питания на выход"), ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON);
            addCommand(tr("ВЫКЛ: подачу питания на выход"), ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF);

            //TODO These two commands are used to separately set output voltage and current (need to be implemented in PowerUnit class Logic)
            //addCommand(tr("УСТАНОВКА: выходное напряжение"), ModuleCommands::SET_SET_VALUE_U);
            //addCommand(tr("УСТАНОВКА: выходной ток"), ModuleCommands::SET_SET_VALUE_I);

            //TODO These two commands are applicable only for TRIPLE device class
            //addCommand(tr("ВКЛ: трекинг"), ModuleCommands::PSC_TRACKING_ON);
            //addCommand(tr("ВЫКЛ: трекинг"), ModuleCommands::PSC_TRACKING_OFF);
        }
        break;

    case ModuleCommands::MKO:
        {
            QMap<QString, QVariant> implicitParamsPsy;
            QMap<QString, QVariant> implicitParamsNu;
            QMap<QString, QVariant> implicitParamsAngleSensorMain;
            QMap<QString, QVariant> implicitParamsAngleSensorReserve;
            QString paramName = sysState->paramName(SystemState::SUBADDRESS);
            implicitParamsPsy[paramName] = QVariant(int(ModuleMKO::PSY_CHANNEL_SUBADDRESS));
            implicitParamsNu[paramName] = QVariant(int(ModuleMKO::NU_CHANNEL_SUBADDRESS));
            implicitParamsAngleSensorMain[paramName] = QVariant(int(ModuleMKO::PS_FROM_MAIN_KIT));
            implicitParamsAngleSensorReserve[paramName] = QVariant(int(ModuleMKO::PS_FROM_RESERVE_KIT));

            addCommand(tr("СТАРТ модуля"), ModuleCommands::START_MKO);
            addCommand(tr("СТОП модуля"), ModuleCommands::STOP_MKO);

            addCommand(tr("ЗАПРОС: командный массив"), ModuleCommands::RECEIVE_COMMAND_ARRAY);
            addCommand(tr("ЗАПРОС: командный массив по линии ψ"), ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("ЗАПРОС: командный массив по линии υ"), ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(tr("ЗАПРОС: тестовый массив"), ModuleCommands::RECEIVE_TEST_ARRAY);
            addCommand(tr("ЗАПРОС: тестовый массив по линии ψ"), ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("ЗАПРОС: тестовый массив по линии υ"), ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(tr("ОТПРАВКА: командный массив"), ModuleCommands::SEND_COMMAND_ARRAY);
            addCommand(tr("ОТПРАВКА: командный массив по линии ψ"), ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("ОТПРАВКА: командный массив по линии υ"), ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(tr("ОТПРАВКА: тестовый массив"), ModuleCommands::SEND_TEST_ARRAY);
            addCommand(tr("ОТПРАВКА: тестовый массив по линии ψ"), ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("ОТПРАВКА: тестовый массив по линии υ"), ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(tr("ОТПРАВКА: подача питания на ДУ (осн. комплект)"), ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorMain);
            addCommand(tr("ОТПРАВКА: подача питания на ДУ (рез. комплект)"), ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorReserve);
        }
        break;

    case ModuleCommands::STM:
        {
            addOKBCommonCommands();

            QMap<QString, QVariant> implicitParams;
            QString channelIDName = sysState->paramName(SystemState::CHANNEL_ID);
            QString powerStateName = sysState->paramName(SystemState::POWER_STATE);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            addCommand(tr("ЗАПРОС: состояние питания основного комплекта БУП"), ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            addCommand(tr("ЗАПРОС: состояние питания резервного комплекта БУП"), ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            addCommand(tr("ЗАПРОС: состояние нагревателей ПНА на линии 1"), ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            addCommand(tr("ЗАПРОС: состояние нагревателей ПНА на линии 2"), ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            addCommand(tr("ЗАПРОС: состояние силового питания"), ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            addCommand(tr("ЗАПРОС: состояние питания основного комплекта МКО"), ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            addCommand(tr("ЗАПРОС: состояние питания резервного комплекта МКО"), ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: основной комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: резервный комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: нагреватели ПНА на линии 1"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: нагреватели ПНА на линии 2"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: подачу силового питания"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: подачу питания на МКО Осн."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("ВКЛ: подачу питания на МКО Рез."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: основной комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: резервный комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: нагреватели ПНА на линии 1"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: нагреватели ПНА на линии 2"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: подачу силового питания"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: подачу питания на МКО Осн."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("ВЫКЛ: подачу питания на МКО Рез."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            int TODO;
            // 1. проверка предохранителей
            // 2. получение телеметрии канала

            //addCommand(tr("Проверить предохранители"), ModuleCommands::GET_PWR_MODULE_FUSE_STATE);
            //addCommand(tr("ПОЛ телеметрию канала"), ModuleCommands::GET_CHANNEL_TELEMETRY);
        }
        break;

    case ModuleCommands::OTD:
        {
            addOKBCommonCommands();

            addCommand(tr("СБРОС: датчиков на линии 1"), ModuleCommands::RESET_LINE_1);
            addCommand(tr("СБРОС: датчиков на линии 2"), ModuleCommands::RESET_LINE_2);
            addCommand(tr("ЗАПРОС: температура с датчиков ПТ-100"), ModuleCommands::GET_TEMPERATURE_PT100);
            addCommand(tr("ЗАПРОС: температура с датчиков DS1820 линия 1"), ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);
            addCommand(tr("ЗАПРОС: температура с датчиков DS1820 линия 2"), ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2);
        }
        break;

    case ModuleCommands::TECH:
        {
            addOKBCommonCommands();
        }
        break;

    default:
        break;
    }

    if (mModuleID == mCommand->module())
    {
        for (int i = 0; i < mCommands->count(); ++i)
        {
            QListWidgetItem* item = mCommands->item(i);
            int commandID = item->data(Qt::UserRole).toInt();

            if (commandID == mCommand->operation())
            {
                const QMap<QString, QVariant>& inputParams = mCommand->inputParams();
                QMap<QString, QVariant> implicitParams = item->data(Qt::UserRole + 1).toMap();
                if (implicitParams.empty())
                {
                    mCommands->setCurrentRow(i);
                    break;
                }

                bool allEqual = true;
                for (auto it = implicitParams.begin(); it != implicitParams.end(); ++it)
                {
                    auto iter = inputParams.find(it.key());
                    if (iter == inputParams.end())
                    {
                        allEqual = false;
                        break;
                    }

                    if (iter.value() != it.value())
                    {
                        allEqual = false;
                        break;
                    }
                }

                if (allEqual)
                {
                    mCommands->setCurrentRow(i);
                    break;
                }
            }
        }
    }
    else
    {
        mCommands->setCurrentRow(0);
    }
}

void CmdActionModuleEditDialog::addOKBCommonCommands()
{
    SystemState* sysState = mCommand->systemState();

    QMap<QString, QVariant> implicitParams;
    QString addressName = sysState->paramName(SystemState::MODULE_ADDRESS);

    implicitParams.clear();
    implicitParams[addressName] = QVariant(int(ModuleCommands::CURRENT));
    addCommand(tr("ЗАПРОС: текущий адрес модуля"), ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    implicitParams.clear();
    implicitParams[addressName] = QVariant(int(ModuleCommands::DEFAULT));
    addCommand(tr("ЗАПРОС: адрес модуля по умолчанию"), ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    addCommand(tr("ЗАПРОС: статусное слово"), ModuleCommands::GET_STATUS_WORD);
    addCommand(tr("СБРОС: ошибки"), ModuleCommands::RESET_ERROR);
}

void CmdActionModuleEditDialog::addCommand(const QString& text, int commandID, const QMap<QString, QVariant>& implicitParams)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);
    item->setData(Qt::UserRole, QVariant(commandID));

    if (!implicitParams.isEmpty())
    {
        item->setData(Qt::UserRole + 1, QVariant(implicitParams));
    }

    mCommands->addItem(item);
}

void CmdActionModuleEditDialog::onCommandChanged(int index)
{
    mInParams->clearContents();
    mOutParams->clearContents();

    if (index == -1)
    {
        mInParams->setRowCount(0);
        mOutParams->setRowCount(0);
        return;
    }

    mCommandID = mCommands->item(index)->data(Qt::UserRole).toInt();
    SystemState* system = mCommand->systemState();
    int inCount = system->paramsCount(mModuleID, mCommandID, true);
    int outCount = system->paramsCount(mModuleID, mCommandID, false);

    mInParams->setRowCount(inCount);
    mOutParams->setRowCount(outCount);

    mInParams->setVisible(inCount > 0);
    mOutParams->setVisible(outCount > 0);

    for (int i = 0; i < inCount; ++i)
    {
        // param name
        QString name = system->paramName(mModuleID, mCommandID, i, true);
        QLabel* text = new QLabel(mInParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mInParams->setCellWidget(i, 0, text);

        // "use variable input" button
        QCheckBox* varSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 1, varSelectBtn);

        // variable selector
        QComboBox* comboBox = new QComboBox(mInParams);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variablesData().keys());
        mInParams->setCellWidget(i, 2, comboBox);

        // "use direct value input" button
        QCheckBox* valueSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 3, valueSelectBtn);

        // variable selector
        QLineEdit* valueEdit = new QLineEdit(mInParams);
        valueEdit->setValidator(new QDoubleValidator(mInParams));
        valueEdit->setText("0");
        mInParams->setCellWidget(i, 4, valueEdit);

        bool isVariable = false; // by default command input params are directly set values

        if (mModuleID == mCommand->module() && mCommandID == mCommand->operation()) // already set command
        {
            const QMap<QString, QVariant>& inputParams = mCommand->inputParams();
            auto it = inputParams.find(name);
            if (it != inputParams.end() && !system->isImplicit(name))
            {
                if (it.value().type() == QMetaType::QString)
                {
                    int index = comboBox->findText(it.value().toString());
                    if (index != -1)
                    {
                        comboBox->setCurrentIndex(index);
                        isVariable = true;
                    }
                }
                else //if (it.value().type() == QMetaType::Double)
                {
                    valueEdit->setText(it.value().toString());
                }
            }
        }

        connect(valueSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));
        connect(varSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));

        valueEdit->setEnabled(!isVariable);
        valueSelectBtn->setChecked(!isVariable);
        comboBox->setEnabled(isVariable);
        varSelectBtn->setChecked(isVariable);
    }

    QSet<QString> generatedVariableNames;

    for (int i = 0; i < outCount; ++i)
    {
        QString name = system->paramName(mModuleID, mCommandID, i, false);
        QLabel* text = new QLabel(mOutParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mOutParams->setCellWidget(i, 0, text);

        QComboBox* comboBox = new QComboBox(mOutParams);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variablesData().keys());
        mOutParams->setCellWidget(i, 1, comboBox);

        if (mModuleID == mCommand->module() && mCommandID == mCommand->operation()) // already set command
        {
            const QMap<QString, QVariant>& outputParams = mCommand->outputParams();
            auto it = outputParams.find(name);
            if (it != outputParams.end() && !system->isImplicit(name))
            {
                int index = comboBox->findText(it.value().toString()); // output params are always variables
                if (index != -1) // add default variable name, corresponding to this paramID
                {
                    comboBox->setCurrentIndex(index);
                }
            }
        }
        else
        {
            QString defaultParamName = system->paramDefaultVarName(system->paramID(name));
            QString tmp = mCommand->moduleName() + "_" + defaultParamName;

            int index = 1;
            while (generatedVariableNames.contains(tmp))
            {
                tmp = defaultParamName + QString::number(index);
                ++index;
            }

            generatedVariableNames.insert(tmp);
            comboBox->addItem(tmp);
            comboBox->setCurrentIndex(comboBox->count() - 1);
        }
    }

    mInParams->resizeColumnToContents(1);
    mInParams->resizeColumnToContents(3);

    //adjustSize();
}

void CmdActionModuleEditDialog::onAccept()
{
    if (mCommand)
    {
        QMap<QString, QVariant> input;
        QMap<QString, QVariant> output;

        SystemState* system = mCommand->systemState();
        int inCount = system->paramsCount(mModuleID, mCommandID, true);
        int outCount = system->paramsCount(mModuleID, mCommandID, false);

        for (int i = 0; i < inCount; ++i)
        {
            QLabel* label = qobject_cast<QLabel*>(mInParams->cellWidget(i, 0));
            QString name;
            if (label)
            {
                name = label->text();
            }

            QCheckBox* varSelectBtn = qobject_cast<QCheckBox*>(mInParams->cellWidget(i, 1));
            //QCheckBox* valueSelectBtn = qobject_cast<QCheckBox*>(mInParams->cellWidget(i, 3));

            bool variableChecked = varSelectBtn->isChecked();
            //bool valueChecked = valueSelectBtn->isChecked();

            if (variableChecked)
            {
                QComboBox* comboBox = qobject_cast<QComboBox*>(mInParams->cellWidget(i, 2));
                if (comboBox)
                {
                    input[name] = comboBox->currentText();
                }
            }
            else
            {
                QLineEdit* valueText = qobject_cast<QLineEdit*>(mInParams->cellWidget(i, 4));
                if (valueText)
                {
                    input[name] = valueText->text().toDouble();
                }
            }
        }

        for (int i = 0; i < outCount; ++i)
        {
            QString name;
            QLabel* label = qobject_cast<QLabel*>(mOutParams->cellWidget(i, 0));
            if (label)
            {
                name = label->text();
            }

            QComboBox* comboBox = qobject_cast<QComboBox*>(mOutParams->cellWidget(i, 1));
            if (comboBox)
            {
                QString variableName = comboBox->currentText();
                VariableController* vc = mCommand->variableController();
                auto it = vc->variablesData().find(variableName);
                if (it == vc->variablesData().end())
                {
                    vc->addVariable(variableName, 0);
                    QString desc = system->paramDefaultDesc(system->paramID(name));
                    vc->setDescription(variableName, desc);
                }

                output[name] = variableName;
            }
        }

        // add implicit params to command input params
        QListWidgetItem* item = mCommands->currentItem();
        QMap<QString,QVariant> implicitParams = item->data(Qt::UserRole + 1).toMap();
        for (auto it = implicitParams.begin(); it != implicitParams.end(); ++it)
        {
            input[it.key()] = it.value();
        }

        mCommand->setParams((ModuleCommands::ModuleID)mModuleID, (ModuleCommands::CommandID)mCommandID, input, output);
        mConsoleTextWidget->saveCommand();
    }

    accept();
}

void CmdActionModuleEditDialog::onCheckBoxStateChanged(int state)
{
    QCheckBox* changedBox = qobject_cast<QCheckBox*>(QObject::sender());
    if (!changedBox)
    {
        LOG_ERROR(QString("Widget not found"));
        return;
    }

    for (int row = 0; row < mInParams->rowCount(); row++)
    {
        QCheckBox* varSelectBox = qobject_cast<QCheckBox*>(mInParams->cellWidget(row, 1));
        QComboBox* varBox = qobject_cast<QComboBox*>(mInParams->cellWidget(row, 2));
        QCheckBox* valueSelectBox = qobject_cast<QCheckBox*>(mInParams->cellWidget(row, 3));
        QLineEdit* valueEdit = qobject_cast<QLineEdit*>(mInParams->cellWidget(row, 4));

        if (varSelectBox == changedBox || valueSelectBox == changedBox)
        {
            varSelectBox->blockSignals(true);
            valueSelectBox->blockSignals(true);

            bool varBoxSelected = (varSelectBox == changedBox) && (state == Qt::Checked);
            bool valueEditSelected = (valueSelectBox == changedBox) && (state == Qt::Checked);
            bool varBoxUnselected = (varSelectBox == changedBox) && (state == Qt::Unchecked);
            bool valueEditUnselected = (valueSelectBox == changedBox) && (state == Qt::Unchecked);

            if (varSelectBox == changedBox)
            {
                valueSelectBox->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
            }
            else if (valueSelectBox == changedBox)
            {
                varSelectBox->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
            }

            varBox->setEnabled(varBoxSelected || valueEditUnselected);
            valueEdit->setEnabled(valueEditSelected || varBoxUnselected);

            varSelectBox->blockSignals(false);
            valueSelectBox->blockSignals(false);
            break;
        }
    }
}
