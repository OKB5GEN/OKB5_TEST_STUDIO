#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/logger/Logger.h"

CmdActionModuleEditDialog::CmdActionModuleEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Module operation"));

    //adjustSize();
    //setFixedSize(sizeHint());

    setFixedSize(QSize(800, 400));
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

    //layout->addWidget(mModules, 0, 0, 10, 4);
    layout->addWidget(mModules, 0, 0);
    //mModules->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    mCommands = new QListWidget(this);
    //layout->addWidget(mCommands, 0, 4, 10, 4);
    layout->addWidget(mCommands, 0, 1);
    //mCommands->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    mInParams = new QTableWidget(this);
    QStringList headers;
    headers.append(tr("Вх.Параметр"));
    headers.append(tr(""));
    headers.append(tr("Переменная"));
    headers.append(tr(""));
    headers.append(tr("Значение"));
    mInParams->setColumnCount(headers.size());
    mInParams->setHorizontalHeaderLabels(headers);
    //layout->addWidget(mInParams, 0, 8, 5, 4);
    layout->addWidget(mInParams, 0, 2);
    //mInParams->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Вых.Параметр"));
    outHeaders.append(tr("Переменная"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
    //layout->addWidget(mOutParams, 5, 8, 5, 4);
    layout->addWidget(mOutParams, 0, 3);
    //mOutParams->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    //layout->addWidget(buttonBox, 11, 5, 1, 2);
    layout->addWidget(buttonBox, 1, 1);

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
    }
}

void CmdActionModuleEditDialog::onModuleChanged(int index)
{
    mModuleID = index;
    mCommands->clear();

    SystemState* sysState = mCommand->systemState();

    // "Abstract" module commands
    addCommand(tr("Получить статус"), ModuleCommands::GET_MODULE_STATUS);
    addCommand(tr("Установить логический статус"), ModuleCommands::SET_MODULE_LOGIC_STATUS);

    // module changed -> update command list for this module
    switch (index)
    {
    case ModuleCommands::POWER_UNIT_BUP:
    case ModuleCommands::POWER_UNIT_PNA:
        {
            // "getters" (just gathering current device state)
            addCommand(tr("Получить текущий ток и напряжение"), ModuleCommands::GET_VOLTAGE_AND_CURRENT);
            addCommand(tr("Получить класс устройства"), ModuleCommands::GET_DEVICE_CLASS);
            addCommand(tr("Получить номинальное напряжение"), ModuleCommands::GET_NOMINAL_VOLTAGE);
            addCommand(tr("Получить номинальный ток"), ModuleCommands::GET_NOMINAL_CURRENT);
            addCommand(tr("Получить номинальную мощность"), ModuleCommands::GET_NOMINAL_POWER);
            addCommand(tr("Получить напряжение отсечки"), ModuleCommands::GET_OVP_THRESHOLD);
            addCommand(tr("Получить ток отсечки"), ModuleCommands::GET_OCP_THRESHOLD);

            // "setters" (changing current device state)
            addCommand(tr("Установить выходное напряжение (максимальная мощность)"), ModuleCommands::SET_VOLTAGE_AND_CURRENT);
            addCommand(tr("Установить напряжение отсечки"), ModuleCommands::SET_OVP_THRESHOLD);
            addCommand(tr("Установить ток отсечки"), ModuleCommands::SET_OCP_THRESHOLD);
            addCommand(tr("Сбросить ошибки"), ModuleCommands::PSC_ACKNOWLEDGE_ALARMS);
            addCommand(tr("Переключить в режим удаленного управления"), ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL);
            addCommand(tr("Переключить в режим ручного управления"), ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL);
            addCommand(tr("Включить подачу питания на выход"), ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON);
            addCommand(tr("Выключить подачу питания на выход"), ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF);

            //TODO These two commands are used to separately set output voltage and current (need to be implemented in PowerUnit class Logic)
            //addCommand(tr("Установить выходное напряжение"), ModuleCommands::SET_SET_VALUE_U);
            //addCommand(tr("Установить выходной ток"), ModuleCommands::SET_SET_VALUE_I);

            //TODO These two commands are applicable only for TRIPLE device class
            //addCommand(tr("Включить трекинг"), ModuleCommands::PSC_TRACKING_ON);
            //addCommand(tr("Выключить трекинг"), ModuleCommands::PSC_TRACKING_OFF);
        }
        break;

    case ModuleCommands::MKO:
        {
            addCommand(tr("Принять тестовый массив"), ModuleCommands::SEND_TEST_ARRAY);
            addCommand(tr("Выдать тестовый массив"), ModuleCommands::RECEIVE_TEST_ARRAY);
            addCommand(tr("Принять командный массив"), ModuleCommands::SEND_COMMAND_ARRAY);
            addCommand(tr("Выдать командный массив"), ModuleCommands::RECEIVE_COMMAND_ARRAY);

            QMap<QString, QVariant> implicitParamsPsy;
            QMap<QString, QVariant> implicitParamsNu;
            QString paramName = sysState->paramName(SystemState::SUBADDRESS);
            implicitParamsPsy[paramName] = QVariant(int(ModuleMKO::PSY_CHANNEL_SUBADDRESS));
            implicitParamsNu[paramName] = QVariant(int(ModuleMKO::NU_CHANNEL_SUBADDRESS));

            addCommand(tr("Принять тестовый массив по линии ψ"), ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("Принять тестовый массив по линии υ"), ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(tr("Выдать тестовый массив по линии ψ"), ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("Выдать тестовый массив по линии υ"), ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(tr("Принять командный массив по линии ψ"), ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("Принять командный массив по линии υ"), ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(tr("Выдать командный массив по линии ψ"), ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(tr("Выдать командный массив по линии υ"), ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(tr("Подать питание на ДУ"), ModuleCommands::SEND_TO_ANGLE_SENSOR);

            // TODO добавить для резервного комплекта (через implicit params видимо)
            addCommand(tr("Старт Осн."), ModuleCommands::START_MKO);
            addCommand(tr("Стоп Осн."), ModuleCommands::STOP_MKO);
        }
        break;

    case ModuleCommands::STM:
        {
            QMap<QString, QVariant> implicitParams;
            QString channelIDName = sysState->paramName(SystemState::CHANNEL_ID);
            QString powerStateName = sysState->paramName(SystemState::POWER_STATE);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить основной комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить основной комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить резервный комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить резервный комплект БУП"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить нагреватели ПНА на линии 1"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить нагреватели ПНА на линии 1"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить нагреватели ПНА на линии 2"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить нагреватели ПНА на линии 2"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить подачу силового питания"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить подачу силового питания"), ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить подачу питания на МКО Осн."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить подачу питания на МКО Осн."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(tr("Включить подачу питания на МКО Рез."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(tr("Выключить подачу питания на МКО Рез."), ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);


            int TODO;
            // 1. проверка предохранителей
            // 2. подключение питания МКО (1, 2) - а нужен ли он выключенный?
            // 3. получение телеметрии канала

            //addCommand(tr("Проверить предохранители"), ModuleCommands::GET_PWR_MODULE_FUSE_STATE);
            //addCommand(tr("Получить телеметрию канала"), ModuleCommands::GET_CHANNEL_TELEMETRY);
            //addCommand(tr("Включить канал СТМ к МКО"), ModuleCommands::SET_MKO_PWR_CHANNEL_STATE);
            //addCommand(tr("Получить состояние канала СТМ к БП"), ModuleCommands::GET_POWER_MODULE_STATE);
            //addCommand(tr("Получить состояние канала СТМ к МКО"), ModuleCommands::GET_MKO_MODULE_STATE);
        }
        break;

    case ModuleCommands::OTD:
        {
            addCommand(tr("Получить температуру с датчиков ПТ-100"), ModuleCommands::GET_TEMPERATURE_PT100);
            addCommand(tr("Получить температуру с датчиков DS1820 линия 1"), ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);
            addCommand(tr("Получить температуру с датчиков DS1820 линия 2"), ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2);
        }
        break;

    case ModuleCommands::TECH://TODO
        {
            //mCommands->addItem(tr("ТЕХ1"));
            //mCommands->addItem(tr("ТЕХ2"));
            //mCommands->addItem(tr("ТЕХ3"));
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
            QString tmp = defaultParamName;

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
