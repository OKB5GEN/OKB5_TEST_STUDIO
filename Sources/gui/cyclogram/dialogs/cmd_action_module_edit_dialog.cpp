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

    setMinimumSize(QSize(1300, 650));
}

CmdActionModuleEditDialog::~CmdActionModuleEditDialog()
{

}

void CmdActionModuleEditDialog::setupUI()
{
    QGridLayout * mainLayout = new QGridLayout(this);
    mValidator = new QDoubleValidator(this);

    QGroupBox* modulesBox = new QGroupBox(tr("Modules"), this);
    QGroupBox* commandsBox = new QGroupBox(tr("Commands"), this);
    QGroupBox* inputBox = new QGroupBox(tr("Input parameters"), this);
    QGroupBox* outputBox = new QGroupBox(tr("Output parameters"), this);

    QVBoxLayout* modulesBoxLayout = new QVBoxLayout();
    QVBoxLayout* commandsBoxLayout = new QVBoxLayout();
    QVBoxLayout* inputBoxLayout = new QVBoxLayout();
    QVBoxLayout* outputBoxLayout = new QVBoxLayout();

    mModules = new QListWidget(this); //TODO names must be in ModuleCommands::ModuleID order
    mModules->addItem(tr("Power unit 1"));
    mModules->addItem(tr("Power unit 2"));
    mModules->addItem(tr("MKO"));
    mModules->addItem(tr("STM"));
    mModules->addItem(tr("OTD"));
    mModules->addItem(tr("Technological"));
    mModules->addItem(tr("DS"));
    modulesBox->setMaximumWidth(150);

    modulesBoxLayout->addWidget(mModules);

    mCommands = new QListWidget(this);
    commandsBoxLayout->addWidget(mCommands);

    mInParams = new QTableWidget(this);
    QStringList headers;
    headers.append(tr("In param"));
    headers.append(tr(""));
    headers.append(tr("Variable"));
    headers.append(tr(""));
    headers.append(tr("Value"));
    mInParams->setColumnCount(headers.size());
    mInParams->setHorizontalHeaderLabels(headers);
    inputBox->setMaximumWidth(400);
    inputBoxLayout->addWidget(mInParams);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Out param"));
    outHeaders.append(tr("Variable"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
    outputBox->setMaximumWidth(250);
    outputBoxLayout->addWidget(mOutParams);

    modulesBox->setLayout(modulesBoxLayout);
    commandsBox->setLayout(commandsBoxLayout);
    inputBox->setLayout(inputBoxLayout);
    outputBox->setLayout(outputBoxLayout);

    mainLayout->addWidget(modulesBox, 0, 0);
    mainLayout->addWidget(commandsBox, 0, 1);
    mainLayout->addWidget(inputBox, 0, 2);
    mainLayout->addWidget(outputBox, 0, 3);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    mainLayout->addWidget(mConsoleTextWidget, 1, 0, 1, 4);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox, 2, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(mModules, SIGNAL(currentRowChanged(int)), this, SLOT(onModuleChanged(int)));
    connect(mCommands, SIGNAL(currentRowChanged(int)), this, SLOT(onCommandChanged(int)));

    setLayout(mainLayout);
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

void CmdActionModuleEditDialog::addPowerUnitCommonCommands()
{
    // "getters" (just gathering current device state)
    addCommand(ModuleCommands::GET_VOLTAGE_AND_CURRENT);
    addCommand(ModuleCommands::GET_DEVICE_CLASS);
    addCommand(ModuleCommands::GET_NOMINAL_VOLTAGE);
    addCommand(ModuleCommands::GET_NOMINAL_CURRENT);
    addCommand(ModuleCommands::GET_NOMINAL_POWER);
    addCommand(ModuleCommands::GET_OVP_THRESHOLD);
    addCommand(ModuleCommands::GET_OCP_THRESHOLD);

    // "setters" (changing current device state)
    addCommand(ModuleCommands::SET_VOLTAGE_AND_CURRENT);
    addCommand(ModuleCommands::SET_OVP_THRESHOLD);
    addCommand(ModuleCommands::SET_OCP_THRESHOLD);
    addCommand(ModuleCommands::PSC_ACKNOWLEDGE_ALARMS);
    addCommand(ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL);
    addCommand(ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL);
    addCommand(ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON);
    addCommand(ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF);

    //TODO These two commands are used to separately set output voltage and current (need to be implemented in PowerUnit class Logic)
    //addCommand(ModuleCommands::SET_SET_VALUE_U);
    //addCommand(ModuleCommands::SET_SET_VALUE_I);

    //TODO These two commands are applicable only for TRIPLE device class
    //addCommand(ModuleCommands::PSC_TRACKING_ON);
    //addCommand(ModuleCommands::PSC_TRACKING_OFF);
}

void CmdActionModuleEditDialog::onModuleChanged(int index)
{
    mModuleID = index;
    mCommands->clear();

    SystemState* sysState = mCommand->systemState();

    // "Abstract" module commands
    addCommand(ModuleCommands::GET_MODULE_STATUS);
    addCommand(ModuleCommands::SET_MODULE_LOGIC_STATUS);

    // module changed -> update command list for this module
    switch (index)
    {
    case ModuleCommands::POWER_UNIT_BUP:
        {
            addPowerUnitCommonCommands();

            // hack for power supply commands mofin from STM to Power Unit >>>
            QMap<QString, QVariant> implicitParams;
            QString channelIDName = sysState->paramName(SystemState::CHANNEL_ID);
            QString powerStateName = sysState->paramName(SystemState::POWER_STATE);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_MAIN));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::BUP_RESERVE));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            addCommand(ModuleCommands::GET_FUSE_STATE); // TODO for fuse id 1, 2, 3, 4 only?
        }
        break;

    case ModuleCommands::POWER_UNIT_PNA:
        {
            addPowerUnitCommonCommands();

            // hack for power supply commands mofin from STM to Power Unit >>>
            QMap<QString, QVariant> implicitParams;
            QString channelIDName = sysState->paramName(SystemState::CHANNEL_ID);
            QString powerStateName = sysState->paramName(SystemState::POWER_STATE);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            addCommand(ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            addCommand(ModuleCommands::GET_FUSE_STATE); // TODO for fuse id 5, 6, 7, 8 only?
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

            addCommand(ModuleCommands::START_MKO);
            addCommand(ModuleCommands::STOP_MKO);

            addCommand(ModuleCommands::RECEIVE_COMMAND_ARRAY);
            addCommand(ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(ModuleCommands::RECEIVE_TEST_ARRAY);
            addCommand(ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(ModuleCommands::SEND_COMMAND_ARRAY);
            addCommand(ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(ModuleCommands::SEND_TEST_ARRAY);
            addCommand(ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorMain);
            addCommand(ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorReserve);

            // hack for power supply commands mofin from STM to MKO >>>
            QMap<QString, QVariant> implicitParams;
            QString channelIDName = sysState->paramName(SystemState::CHANNEL_ID);
            QString powerStateName = sysState->paramName(SystemState::POWER_STATE);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            addCommand(ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            addCommand(ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_ON));
            addCommand(ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_1));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[channelIDName] = QVariant(int(ModuleCommands::MKO_2));
            implicitParams[powerStateName] = QVariant(int(ModuleCommands::POWER_OFF));
            addCommand(ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);
        }
        break;

    case ModuleCommands::STM:
        {
            addOKBCommonCommands();

            addCommand(ModuleCommands::GET_CHANNEL_TELEMETRY);
        }
        break;

    case ModuleCommands::OTD:
        {
            QMap<QString, QVariant> implicitParams1;
            QMap<QString, QVariant> implicitParams2;
            QString sensorID = sysState->paramName(SystemState::SENSOR_NUMBER);
            implicitParams1[sensorID] = QVariant(int(1));
            implicitParams2[sensorID] = QVariant(int(2));

            addOKBCommonCommands();

            addCommand(ModuleCommands::RESET_LINE_1);
            addCommand(ModuleCommands::RESET_LINE_2);
            addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_1);
            addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_2);
            addCommand(ModuleCommands::START_MEASUREMENT_LINE_1);
            addCommand(ModuleCommands::START_MEASUREMENT_LINE_2);
            addCommand(ModuleCommands::GET_TEMPERATURE_PT100, implicitParams1);
            addCommand(ModuleCommands::GET_TEMPERATURE_PT100, implicitParams2);
            addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);
            addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2);

            //3. Чтение адресов на линии 1 (ОТД) //TODO
            //4. Чтение адресов на линии 2 (ОТД) //TODO
        }
        break;

    case ModuleCommands::DRIVE_SIMULATOR:
        {
            addOKBCommonCommands();

            addCommand(ModuleCommands::RESET_LINE_1);
            addCommand(ModuleCommands::GET_DS1820_COUNT_LINE_1);
            addCommand(ModuleCommands::START_MEASUREMENT_LINE_1);
            addCommand(ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);

            //10. Чтение адресов на линии (ИП) //TODO
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

    // if current command module corresponds to currently opened module tab, select command in the list
    if (mModuleID != mCommand->module())
    {
        mCommands->setCurrentRow(0);
        return;
    }

    for (int i = 0; i < mCommands->count(); ++i)
    {
        QListWidgetItem* item = mCommands->item(i);
        int commandID = item->data(Qt::UserRole).toInt();

        if (commandID != mCommand->operation())
        {
            continue;
        }

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

void CmdActionModuleEditDialog::addOKBCommonCommands()
{
    SystemState* sysState = mCommand->systemState();

    QMap<QString, QVariant> implicitParams;
    QString addressName = sysState->paramName(SystemState::MODULE_ADDRESS);

    implicitParams.clear();
    implicitParams[addressName] = QVariant(int(ModuleCommands::CURRENT));
    addCommand(ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    implicitParams.clear();
    implicitParams[addressName] = QVariant(int(ModuleCommands::DEFAULT));
    addCommand(ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    addCommand(ModuleCommands::GET_STATUS_WORD);
    addCommand(ModuleCommands::RESET_ERROR);

    //TODO
    //1. Soft reset
    //2. Get software version
    //3. Echo
}

void CmdActionModuleEditDialog::addCommand(int commandID, const QMap<QString, QVariant>& implicitParams)
{
    QString fullText = mCommand->commandName(commandID, implicitParams, true);
    fullText += " (";
    fullText += mCommand->commandName(commandID, implicitParams);
    fullText += ")";

    QListWidgetItem* item = new QListWidgetItem();
    item->setText(fullText);
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

    //mInParams->setVisible(inCount > 0);
    //mOutParams->setVisible(outCount > 0);

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
        comboBox->installEventFilter(this);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variablesData().keys());
        mInParams->setCellWidget(i, 2, comboBox);

        // "use direct value input" button
        QCheckBox* valueSelectBtn = new QCheckBox(mInParams);
        mInParams->setCellWidget(i, 3, valueSelectBtn);

        // variable selector
        QLineEdit* valueEdit = new QLineEdit(mInParams);
        valueEdit->setValidator(new QDoubleValidator(mInParams));

        // direct value selection
        QString defaultValue = "0";
        switch (mCommandID)
        {
        case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1:
        case ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2:
        case ModuleCommands::GET_TEMPERATURE_PT100:
        case ModuleCommands::GET_CHANNEL_TELEMETRY:
        case ModuleCommands::GET_FUSE_STATE:
        case ModuleCommands::GET_MKO_POWER_CHANNEL_STATE:
        case ModuleCommands::SET_MKO_POWER_CHANNEL_STATE:
        case ModuleCommands::GET_POWER_CHANNEL_STATE:
        case ModuleCommands::SET_POWER_CHANNEL_STATE:
        case ModuleCommands::GET_DS1820_ADDR_LINE_1:
        case ModuleCommands::GET_DS1820_ADDR_LINE_2:
            defaultValue = "1";
            break;
        default:
            break;
        }
        valueEdit->setText(defaultValue);

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
        comboBox->installEventFilter(this);
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
            QString tmp = CmdActionModule::moduleName(mModuleID, false) + "_" + defaultParamName;

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

bool CmdActionModuleEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(obj))
    {
        return true; // do not process wheel events if combo box is not "expanded/opened"
    }
    else
    {
        return QObject::eventFilter(obj, event); // standard event processing
    }
}
