#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/logger/Logger.h"
#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/gui/cyclogram/dialogs/text_edit_dialog.h"

#include <QtWidgets>
#include <QSettings>

namespace
{
    static const QString SETTING_LAST_OPENED_MODULE = "LastOpenedModule";
    static const char* PREV_INDEX = "PrevIndex";
}

CmdActionModuleEditDialog::CmdActionModuleEditDialog(QWidget * parent):
    RestorableDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Module operation"));

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

    mSetCommands = new QListWidget(this);
    mGetCommands = new QListWidget(this);
    commandsBoxLayout->addWidget(new QLabel(tr("Set module param"), this));
    commandsBoxLayout->addWidget(mSetCommands);
    commandsBoxLayout->addWidget(new QLabel(tr("Get module param"), this));
    commandsBoxLayout->addWidget(mGetCommands);

    mInParams = new QTableWidget(this);
    QStringList headers;
    headers.append(tr("In param"));
    headers.append(tr(""));
    headers.append(tr("Variable"));
    headers.append(tr(""));
    headers.append(tr("Value"));
    mInParams->setColumnCount(headers.size());
    mInParams->setHorizontalHeaderLabels(headers);
    inputBoxLayout->addWidget(mInParams);

    mOutParams = new QTableWidget(this);
    QStringList outHeaders;
    outHeaders.append(tr("Out param"));
    outHeaders.append(tr("Variable"));
    mOutParams->setColumnCount(outHeaders.size());
    mOutParams->setHorizontalHeaderLabels(outHeaders);
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
    connect(mSetCommands, SIGNAL(currentRowChanged(int)), this, SLOT(onCommandChanged(int)));
    connect(mGetCommands, SIGNAL(currentRowChanged(int)), this, SLOT(onCommandChanged(int)));

    setLayout(mainLayout);
}

void CmdActionModuleEditDialog::setCommand(CmdActionModule* command)
{
    mCommand = command;

    if (!mCommand)
    {
        return;
    }

    int index = 0;
    if (mCommand->hasError())
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        int module = settings.value(SETTING_LAST_OPENED_MODULE, "-1").toInt();
        if (module != -1 && module < mModules->count())
        {
            index = module;
        }
    }
    else
    {
        index = mCommand->module();
    }

    mModules->setCurrentRow(index);
    mConsoleTextWidget->setCommand(mCommand);

    readSettings();
}

void CmdActionModuleEditDialog::addPowerUnitCommonCommands(uint32_t moduleID)
{
    // "getters" (just gathering current device state)
    addCommand(moduleID, ModuleCommands::GET_VOLTAGE_AND_CURRENT);
    addCommand(moduleID, ModuleCommands::GET_DEVICE_CLASS);
    addCommand(moduleID, ModuleCommands::GET_NOMINAL_VOLTAGE);
    addCommand(moduleID, ModuleCommands::GET_NOMINAL_CURRENT);
    addCommand(moduleID, ModuleCommands::GET_NOMINAL_POWER);
    addCommand(moduleID, ModuleCommands::GET_OVP_THRESHOLD);
    addCommand(moduleID, ModuleCommands::GET_OCP_THRESHOLD);

    // "setters" (changing current device state)
    addCommand(moduleID, ModuleCommands::SET_VOLTAGE_AND_CURRENT);
    addCommand(moduleID, ModuleCommands::SET_OVP_THRESHOLD);
    addCommand(moduleID, ModuleCommands::SET_OCP_THRESHOLD);
    addCommand(moduleID, ModuleCommands::PSC_ACKNOWLEDGE_ALARMS);
    addCommand(moduleID, ModuleCommands::PSC_SWITCH_TO_REMOTE_CTRL);
    addCommand(moduleID, ModuleCommands::PSC_SWITCH_TO_MANUAL_CTRL);
    addCommand(moduleID, ModuleCommands::PSC_SWITCH_POWER_OUTPUT_ON);
    addCommand(moduleID, ModuleCommands::PSC_SWITCH_POWER_OUTPUT_OFF);

    //TODO These two commands are used to separately set output voltage and current (need to be implemented in PowerUnit class Logic)
    //addCommand(moduleID, ModuleCommands::SET_SET_VALUE_U);
    //addCommand(moduleID, ModuleCommands::SET_SET_VALUE_I);

    //TODO These two commands are applicable only for TRIPLE device class
    //addCommand(moduleID, ModuleCommands::PSC_TRACKING_ON);
    //addCommand(moduleID, ModuleCommands::PSC_TRACKING_OFF);
}

void CmdActionModuleEditDialog::onModuleChanged(int index)
{
    mModuleID = index;
    mSetCommands->clear();
    mGetCommands->clear();

    //SystemState* sysState = mCommand->systemState();

    // "Abstract" module commands
    addCommand(mModuleID, ModuleCommands::GET_MODULE_STATUS);
    addCommand(mModuleID, ModuleCommands::SET_MODULE_LOGIC_STATUS);

    // module changed -> update command list for this module
    switch (mModuleID)
    {
    case ModuleCommands::POWER_UNIT_BUP:
        {
            addPowerUnitCommonCommands(mModuleID);

            // hack for power supply commands moving from STM to Power Unit >>>
            QMap<uint32_t, QVariant> implicitParams;

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::BUP_MAIN));
            addCommand(mModuleID, ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::BUP_RESERVE));
            addCommand(mModuleID, ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::BUP_MAIN));
            addCommand(mModuleID, ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::BUP_RESERVE));
            addCommand(mModuleID, ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            addCommand(mModuleID, ModuleCommands::GET_FUSE_STATE); // TODO for fuse id 1, 2, 3, 4 only?
        }
        break;

    case ModuleCommands::POWER_UNIT_PNA:
        {
            addPowerUnitCommonCommands(mModuleID);

            // hack for power supply commands mofin from STM to Power Unit >>>
            QMap<uint32_t, QVariant> implicitParams;

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            addCommand(mModuleID, ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            addCommand(mModuleID, ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            addCommand(mModuleID, ModuleCommands::GET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::HEATER_LINE_1));
            addCommand(mModuleID, ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::HEATER_LINE_2));
            addCommand(mModuleID, ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::DRIVE_CONTROL));
            addCommand(mModuleID, ModuleCommands::SET_POWER_CHANNEL_STATE, implicitParams);

            addCommand(mModuleID, ModuleCommands::GET_FUSE_STATE); // TODO for fuse id 5, 6, 7, 8 only?
        }
        break;

    case ModuleCommands::MKO:
        {
            QMap<uint32_t, QVariant> implicitParamsPsy;
            QMap<uint32_t, QVariant> implicitParamsNu;
            QMap<uint32_t, QVariant> implicitParamsAngleSensorMain;
            QMap<uint32_t, QVariant> implicitParamsAngleSensorReserve;

            implicitParamsPsy[SystemState::SUBADDRESS] = QVariant(int(ModuleMKO::PSY_CHANNEL_SUBADDRESS));
            implicitParamsNu[SystemState::SUBADDRESS] = QVariant(int(ModuleMKO::NU_CHANNEL_SUBADDRESS));
            implicitParamsAngleSensorMain[SystemState::SUBADDRESS] = QVariant(int(ModuleMKO::PS_FROM_MAIN_KIT));
            implicitParamsAngleSensorReserve[SystemState::SUBADDRESS] = QVariant(int(ModuleMKO::PS_FROM_RESERVE_KIT));

            addCommand(mModuleID, ModuleCommands::START_MKO);
            addCommand(mModuleID, ModuleCommands::STOP_MKO);

            addCommand(mModuleID, ModuleCommands::RECEIVE_COMMAND_ARRAY);
            addCommand(mModuleID, ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(mModuleID, ModuleCommands::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(mModuleID, ModuleCommands::RECEIVE_TEST_ARRAY);
            addCommand(mModuleID, ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(mModuleID, ModuleCommands::RECEIVE_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(mModuleID, ModuleCommands::SEND_COMMAND_ARRAY);
            addCommand(mModuleID, ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(mModuleID, ModuleCommands::SEND_COMMAND_ARRAY_FOR_CHANNEL, implicitParamsNu);
            addCommand(mModuleID, ModuleCommands::SEND_TEST_ARRAY);
            addCommand(mModuleID, ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsPsy);
            addCommand(mModuleID, ModuleCommands::SEND_TEST_ARRAY_FOR_CHANNEL, implicitParamsNu);

            addCommand(mModuleID, ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorMain);
            addCommand(mModuleID, ModuleCommands::SEND_TO_ANGLE_SENSOR, implicitParamsAngleSensorReserve);

            // hack for power supply commands mofin from STM to MKO >>>
            QMap<uint32_t, QVariant> implicitParams;

            implicitParams.clear();
            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::MKO_1));
            addCommand(mModuleID, ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::MKO_2));
            addCommand(mModuleID, ModuleCommands::GET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::MKO_1));
            addCommand(mModuleID, ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);

            implicitParams.clear();
            implicitParams[SystemState::CHANNEL_ID] = QVariant(int(ModuleCommands::MKO_2));
            addCommand(mModuleID, ModuleCommands::SET_MKO_POWER_CHANNEL_STATE, implicitParams);
        }
        break;

    case ModuleCommands::STM:
        {
            addOKBCommonCommands(mModuleID);

            addCommand(mModuleID, ModuleCommands::GET_CHANNEL_TELEMETRY);
        }
        break;

    case ModuleCommands::OTD:
        {
            QMap<uint32_t, QVariant> implicitParams1;
            QMap<uint32_t, QVariant> implicitParams2;
            implicitParams1[SystemState::SENSOR_NUMBER] = QVariant(int(1));
            implicitParams2[SystemState::SENSOR_NUMBER] = QVariant(int(2));

            addOKBCommonCommands(mModuleID);

            addCommand(mModuleID, ModuleCommands::RESET_LINE_1);
            addCommand(mModuleID, ModuleCommands::RESET_LINE_2);
            addCommand(mModuleID, ModuleCommands::GET_DS1820_COUNT_LINE_1);
            addCommand(mModuleID, ModuleCommands::GET_DS1820_COUNT_LINE_2);
            addCommand(mModuleID, ModuleCommands::START_MEASUREMENT_LINE_1);
            addCommand(mModuleID, ModuleCommands::START_MEASUREMENT_LINE_2);
            addCommand(mModuleID, ModuleCommands::GET_TEMPERATURE_PT100, implicitParams1);
            addCommand(mModuleID, ModuleCommands::GET_TEMPERATURE_PT100, implicitParams2);
            addCommand(mModuleID, ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);
            addCommand(mModuleID, ModuleCommands::GET_TEMPERATURE_DS1820_LINE_2);

            //3. Чтение адресов на линии 1 (ОТД) //TODO
            //4. Чтение адресов на линии 2 (ОТД) //TODO
        }
        break;

    case ModuleCommands::DRIVE_SIMULATOR:
        {
            addOKBCommonCommands(mModuleID);

            addCommand(mModuleID, ModuleCommands::RESET_LINE_1);
            addCommand(mModuleID, ModuleCommands::GET_DS1820_COUNT_LINE_1);
            addCommand(mModuleID, ModuleCommands::START_MEASUREMENT_LINE_1);
            addCommand(mModuleID, ModuleCommands::GET_TEMPERATURE_DS1820_LINE_1);

            //10. Чтение адресов на линии (ИП) //TODO
        }
        break;

    case ModuleCommands::TECH:
        {
            addOKBCommonCommands(mModuleID);
        }
        break;

    default:
        break;
    }

    // if current command module corresponds to currently opened module tab, select command in the list
    if (mModuleID != mCommand->module())
    {
        mSetCommands->setCurrentRow(0);
        return;
    }

    ModuleCommands::CommandID cmd = ModuleCommands::CommandID(mCommand->operation());
    QListWidget* commandsWidget = SystemState::isSetter(cmd) ? mSetCommands : mGetCommands;

    for (int i = 0; i < commandsWidget->count(); ++i)
    {
        QListWidgetItem* item = commandsWidget->item(i);
        int commandID = item->data(Qt::UserRole).toInt();

        if (commandID != mCommand->operation())
        {
            continue;
        }

        QMetaEnum params = QMetaEnum::fromType<SystemState::ParamID>();

        const QMap<uint32_t, QVariant>& inputParams = mCommand->inputParams();
        QMap<QString, QVariant> implicitParams = item->data(Qt::UserRole + 1).toMap();
        if (implicitParams.empty())
        {
            commandsWidget->setCurrentRow(i);
            break;
        }

        bool allEqual = true;
        for (auto it = implicitParams.begin(); it != implicitParams.end(); ++it)
        {
            uint32_t id = params.keyToValue(qPrintable(it.key()));
            auto iter = inputParams.find(id);
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
            commandsWidget->setCurrentRow(i);
            break;
        }
    }
}

void CmdActionModuleEditDialog::addOKBCommonCommands(uint32_t moduleID)
{
    QMap<uint32_t, QVariant> implicitParams;

    implicitParams[SystemState::MODULE_ADDRESS] = QVariant(int(ModuleCommands::CURRENT));
    addCommand(moduleID, ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    implicitParams[SystemState::MODULE_ADDRESS] = QVariant(int(ModuleCommands::DEFAULT));
    addCommand(moduleID, ModuleCommands::GET_MODULE_ADDRESS, implicitParams);

    addCommand(moduleID, ModuleCommands::GET_STATUS_WORD);
    addCommand(moduleID, ModuleCommands::RESET_ERROR);

    //TODO
    //1. Soft reset
    //2. Get software version
    //3. Echo
}

void CmdActionModuleEditDialog::addCommand(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& implicitInputParams)
{
    QString fullText = CmdActionModule::commandName(moduleID, commandID, implicitInputParams);

    QListWidgetItem* item = new QListWidgetItem();
    item->setText(fullText);
    item->setData(Qt::UserRole, QVariant(commandID));

    if (!implicitInputParams.isEmpty())
    {
        QMetaEnum params = QMetaEnum::fromType<SystemState::ParamID>();
        QMap<QString, QVariant> impl;

        for (auto it = implicitInputParams.begin(); it != implicitInputParams.end(); ++it)
        {
            QString str = params.valueToKey(it.key());
            impl[str] = it.value();
        }

        item->setData(Qt::UserRole + 1, QVariant(impl));
    }

    if (SystemState::isSetter(ModuleCommands::CommandID(commandID)))
    {
        mSetCommands->addItem(item);
    }
    else
    {
        mGetCommands->addItem(item);
    }
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

    QListWidget* commandsListWidget = Q_NULLPTR;
    if (QObject::sender() == mSetCommands)
    {
        commandsListWidget = mSetCommands;
        mGetCommands->clearSelection();
    }
    else if (QObject::sender() == mGetCommands)
    {
        commandsListWidget = mGetCommands;
        mSetCommands->clearSelection();
    }

    if (!commandsListWidget)
    {
        LOG_ERROR(QString("CmdActionModuleEditDialog::onCommandChanged: invalid command sender!"));
        return;
    }

    mCommandID = commandsListWidget->item(index)->data(Qt::UserRole).toInt();
    SystemState* system = mCommand->systemState();

    QList<uint32_t> inParams = system->inputParams(mModuleID, mCommandID).toList();
    QList<uint32_t> outParams = system->outputParams(mModuleID, mCommandID).toList();

    std::sort(inParams.begin(), inParams.end());
    std::sort(outParams.begin(), outParams.end());

    mInParams->setRowCount(inParams.size());
    mOutParams->setRowCount(outParams.size());

    int i = 0;
    for (auto it = inParams.begin(); it != inParams.end(); ++it)
    {
        QString name = system->paramData(SystemState::ParamID(*it)).name;

        QTableWidgetItem* item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, *it);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        mInParams->setItem(i, 0, item);

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
            const QMap<uint32_t, QVariant>& commandInputParams = mCommand->inputParams();
            auto iter = commandInputParams.find(*it);
            if (iter != commandInputParams.end() && !system->isImplicit(SystemState::ParamID(*it)))
            {
                if (iter.value().type() == QMetaType::QString)
                {
                    int index = comboBox->findText(iter.value().toString());
                    if (index != -1)
                    {
                        comboBox->setCurrentIndex(index);
                        isVariable = true;
                    }
                }
                else //if (iter.value().type() == QMetaType::Double)
                {
                    valueEdit->setText(iter.value().toString());
                }
            }
        }

        connect(valueSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));
        connect(varSelectBtn, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)));

        valueEdit->setEnabled(!isVariable);
        valueSelectBtn->setChecked(!isVariable);
        comboBox->setEnabled(isVariable);
        varSelectBtn->setChecked(isVariable);

        ++i;
    }

    i = 0;
    for (auto it = outParams.begin(); it != outParams.end(); ++it)
    {
        QString name = system->paramData(SystemState::ParamID(*it)).name;

        QTableWidgetItem* item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, *it);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        mOutParams->setItem(i, 0, item);

        QComboBox* comboBox = new QComboBox(mOutParams);
        comboBox->installEventFilter(this);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variablesData().keys());
        mOutParams->setCellWidget(i, 1, comboBox);

        connect(comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onOutVarChanged(const QString&)));

        if (mModuleID == mCommand->module() && mCommandID == mCommand->operation()) // already set command
        {
            const QMap<uint32_t, QVariant>& commandOutputParams = mCommand->outputParams();
            auto iter = commandOutputParams.find(*it);
            if (iter != commandOutputParams.end() && !system->isImplicit(SystemState::ParamID(*it)))
            {
                int index = comboBox->findText(iter.value().toString()); // output params are always variables
                if (index != -1) // add default variable name, corresponding to this paramID
                {
                    comboBox->setCurrentIndex(index);
                    comboBox->setProperty(PREV_INDEX, index);
                }
            }

            comboBox->addItem(TextEditDialog::addVarText()); // add ability to add variable
        }
        else
        {
            QString defaultParamName = system->paramData(SystemState::ParamID(*it)).variable;
            QString defaultVarName = CmdActionModule::moduleName(mModuleID, false) + "_" + defaultParamName;

            comboBox->addItem(defaultVarName);
            comboBox->addItem(TextEditDialog::addVarText());
            comboBox->setCurrentIndex(comboBox->count() - 2); // set default genarated name by default
        }

        ++i;
    }

    mInParams->resizeColumnsToContents();
    mOutParams->resizeColumnsToContents();
}

void CmdActionModuleEditDialog::onAccept()
{
    if (!mCommand)
    {
        LOG_ERROR(QString("No command set"));
        return;
    }

    QMap<uint32_t, QVariant> input;
    QMap<uint32_t, QVariant> output;

    SystemState* system = mCommand->systemState();

    int inCount = mInParams->rowCount();
    int outCount = mOutParams->rowCount();

    for (int i = 0; i < inCount; ++i)
    {
        QTableWidgetItem* item = mInParams->item(i, 0);
        uint32_t id = item->data(Qt::UserRole).toInt();

        QCheckBox* varSelectBtn = qobject_cast<QCheckBox*>(mInParams->cellWidget(i, 1));
        //QCheckBox* valueSelectBtn = qobject_cast<QCheckBox*>(mInParams->cellWidget(i, 3));

        bool variableChecked = varSelectBtn->isChecked();
        //bool valueChecked = valueSelectBtn->isChecked();

        if (variableChecked)
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(mInParams->cellWidget(i, 2));
            if (comboBox)
            {
                input[id] = comboBox->currentText();
            }
        }
        else
        {
            QLineEdit* valueText = qobject_cast<QLineEdit*>(mInParams->cellWidget(i, 4));
            if (valueText)
            {
                input[id] = valueText->text().toDouble();
            }
        }
    }

    for (int i = 0; i < outCount; ++i)
    {
        QTableWidgetItem* item = mOutParams->item(i, 0);
        uint32_t id = item->data(Qt::UserRole).toInt();

        QComboBox* comboBox = qobject_cast<QComboBox*>(mOutParams->cellWidget(i, 1));
        if (comboBox)
        {
            QString variableName = comboBox->currentText();
            VariableController* vc = mCommand->variableController();
            auto it = vc->variablesData().find(variableName);
            if (it == vc->variablesData().end())
            {
                qreal value = 0;
                if (comboBox->currentData().isValid())
                {
                    value = comboBox->currentData().toDouble();
                }

                vc->addVariable(variableName, value);
                QString desc = system->paramData(SystemState::ParamID(id)).description;
                vc->setDescription(variableName, desc);
            }

            output[id] = variableName;
        }
    }

    // add implicit params to command input params
    QList<QListWidgetItem*> items = mSetCommands->selectedItems();
    if (items.isEmpty())
    {
        items = mGetCommands->selectedItems();
    }

    if (items.isEmpty() || items.size() > 1)
    {
        LOG_ERROR(QString("Invalid commands selection"));
        reject();
        return;
    }

    QMetaEnum params = QMetaEnum::fromType<SystemState::ParamID>();
    QMap<QString, QVariant> implicitParams = items.back()->data(Qt::UserRole + 1).toMap();
    for (auto it = implicitParams.begin(); it != implicitParams.end(); ++it)
    {
        uint32_t id = params.keyToValue(qPrintable(it.key()));
        input[id] = it.value();
    }

    mCommand->setParams((ModuleCommands::ModuleID)mModuleID, (ModuleCommands::CommandID)mCommandID, input, output);
    mConsoleTextWidget->saveCommand();

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(SETTING_LAST_OPENED_MODULE, QString::number(mModules->currentRow()));

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

void CmdActionModuleEditDialog::onOutVarChanged(const QString& text)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(QObject::sender());
    if (!comboBox)
    {
        return;
    }

    if (text != TextEditDialog::addVarText())
    {
        int index = comboBox->findText(text);
        if (index != -1)
        {
            comboBox->setProperty(PREV_INDEX, index);
        }

        return;
    }

    TextEditDialog dialog(TextEditDialog::VARIABLE_EDIT, this);
    dialog.setText("NewVariable");
    dialog.setCommand(mCommand);
    int result = dialog.exec();

    QString newVariable = dialog.text();

    if (result != QDialog::Accepted)
    {
        QVariant prevIndex = comboBox->property(PREV_INDEX);
        if (prevIndex.isValid())
        {
            comboBox->setCurrentIndex(prevIndex.toInt()); // revert to previous variable
        }
        else
        {
            comboBox->setCurrentIndex(0);
        }

        return;
    }

    comboBox->blockSignals(true);
    int index = comboBox->count() - 1;
    comboBox->insertItem(index, newVariable, dialog.value());
    comboBox->setCurrentIndex(index);
    comboBox->setProperty(PREV_INDEX, index);
    comboBox->blockSignals(false);
}
