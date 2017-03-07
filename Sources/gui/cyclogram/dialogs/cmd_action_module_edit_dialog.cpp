#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/system/modules/module_mko.h"

CmdActionModuleEditDialog::CmdActionModuleEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR)
{
    setupUI();
    setWindowTitle(tr("Module operation"));

    adjustSize();
    setFixedSize(sizeHint());
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

    layout->addWidget(mModules, 0, 0, 5, 4);

    mCommands = new QListWidget(this);
    layout->addWidget(mCommands, 0, 4, 5, 4);

    mParams = new QTableWidget(this);
    mParams->setColumnCount(2);
    QStringList headers;
    headers.append(tr("Параметр"));
    headers.append(tr("Значение"));
    mParams->setHorizontalHeaderLabels(headers);
    layout->addWidget(mParams, 0, 8, 5, 4);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 5, 5, 1, 2);

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

    // module changed -> update command list for this module
    switch (index)
    {
    case ModuleCommands::POWER_UNIT_BUP:
    case ModuleCommands::POWER_UNIT_PNA:
        {
            addCommand(tr("Установить текущее значение"), ModuleCommands::SET_VOLTAGE_AND_CURRENT);
            addCommand(tr("Получить текущее значение"), ModuleCommands::GET_VOLTAGE_AND_CURRENT);

            //addCommand(tr("Установить ограничение"), ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT);

            //TODO здесь подключение основного/резервного полукомплекта БУП и относится оно скорее всего к POWER_UNIT_BUP только

            QList<int> params;
            params.push_back(ModuleCommands::SET_POWER_STATE);
            params.push_back(1); //params count
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить подачу питания"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_STATE);
            params.push_back(1); //params count
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить подачу питания"), params);
        }
        break;

    case ModuleCommands::MKO:
        {
            //addCommand(tr("Принять тестовый массив"), ModuleMKO::SEND_TEST_ARRAY);
            //addCommand(tr("Выдать тестовый массив"), ModuleMKO::RECEIVE_TEST_ARRAY);
            //addCommand(tr("Принять командный массив"), ModuleMKO::SEND_COMMAND_ARRAY);
            //addCommand(tr("Выдать командный массив"), ModuleMKO::RECEIVE_COMMAND_ARRAY);
            //addCommand(tr("Принять тестовый массив по линии ψ"), ModuleMKO::SEND_TEST_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Принять тестовый массив по линии υ"), ModuleMKO::SEND_TEST_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Выдать тестовый массив по линии ψ"), ModuleMKO::RECEIVE_TEST_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Выдать тестовый массив по линии υ"), ModuleMKO::RECEIVE_TEST_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Принять командный массив по линии ψ"), ModuleMKO::SEND_COMMAND_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Принять командный массив по линии υ"), ModuleMKO::SEND_COMMAND_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Выдать командный массив по линии ψ"), ModuleMKO::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL);
            //addCommand(tr("Выдать командный массив по линии υ"), ModuleMKO::RECEIVE_COMMAND_ARRAY_FOR_CHANNEL);
            addCommand(tr("Подать питание на ДУ"), ModuleMKO::SEND_TO_ANGLE_SENSOR);
        }
        break;

    case ModuleCommands::STM:
        {
            QList<int> params;

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::BUP_MAIN);
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить основной комплект БУП"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::BUP_MAIN);
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить основной комплект БУП"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::BUP_RESERVE);
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить резервный комплект БУП"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::BUP_RESERVE);
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить резервный комплект БУП"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::HEATER_LINE_1);
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить нагреватели ПНА на линии 1"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::HEATER_LINE_1);
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить нагреватели ПНА на линии 1"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::HEATER_LINE_2);
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить нагреватели ПНА на линии 2"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::HEATER_LINE_2);
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить нагреватели ПНА на линии 2"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::DRIVE_CONTROL);
            params.push_back(ModuleCommands::POWER_ON);
            addCommand(tr("Включить подачу силового питания"), params);

            params.clear();
            params.push_back(ModuleCommands::SET_POWER_CHANNEL_STATE);
            params.push_back(2); //params count
            params.push_back(ModuleCommands::DRIVE_CONTROL);
            params.push_back(ModuleCommands::POWER_OFF);
            addCommand(tr("Выключить подачу силового питания"), params);

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
            int paramsCount = item->data(Qt::UserRole + 1).toInt();

            if (commandID == mCommand->operation())
            {
                const QList<int>& implicitParams = mCommand->implicitParams();

                bool allEqual = true;
                for (int j = 0; j < paramsCount; ++j)
                {
                    int param = item->data(Qt::UserRole + j + 2).toInt();
                    if (param != implicitParams[j])
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

void CmdActionModuleEditDialog::addCommand(const QString& text, int commandID)
{
    QList<int> params;
    params.push_back(commandID);
    params.push_back(0);
    addCommand(text, params);
}

void CmdActionModuleEditDialog::addCommand(const QString& text, const QList<int>& params)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);

    for (int i = 0, sz = params.size(); i < sz; ++i)
    {
        item->setData(Qt::UserRole + i, QVariant(params[i]));
    }

    mCommands->addItem(item);
}

void CmdActionModuleEditDialog::onCommandChanged(int index)
{
    mParams->clearContents();

    if (index == -1)
    {
        mParams->setRowCount(0);
        return;
    }

    const QMap<QString, QString>& inputParams = mCommand->inputParams();
    const QMap<QString, QString>& outputParams = mCommand->outputParams();

    mCommandID = mCommands->item(index)->data(Qt::UserRole).toInt();
    SystemState* system = mCommand->systemState();
    int inCount = system->paramsCount(mModuleID, mCommandID, true);
    int outCount = system->paramsCount(mModuleID, mCommandID, false);
    mParams->setRowCount(inCount + outCount);

    for (int i = 0; i < inCount; ++i)
    {
        QString name = system->paramName(mModuleID, mCommandID, i, true);
        QLabel* text = new QLabel(mParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mParams->setCellWidget(i, 0, text);

        QComboBox* comboBox = new QComboBox(mParams);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variables().keys());
        mParams->setCellWidget(i, 1, comboBox);

        if (mModuleID == mCommand->module() && mCommandID == mCommand->operation())
        {
            QMap<QString, QString>::const_iterator it = inputParams.find(name);
            if (it != inputParams.end())
            {
                int index = comboBox->findText(it.value());
                if (index != -1)
                {
                    comboBox->setCurrentIndex(index);
                }
            }
        }
    }

    for (int i = 0; i < outCount; ++i)
    {
        QString name = system->paramName(mModuleID, mCommandID, i, false);
        QLabel* text = new QLabel(mParams);
        text->setTextInteractionFlags(Qt::NoTextInteraction);
        text->setText(name);
        mParams->setCellWidget(i + inCount, 0, text);

        QComboBox* comboBox = new QComboBox(mParams);
        VariableController* vc = mCommand->variableController();
        comboBox->addItems(vc->variables().keys());
        mParams->setCellWidget(i + inCount, 1, comboBox);

        if (mModuleID == mCommand->module() && mCommandID == mCommand->operation())
        {
            QMap<QString, QString>::const_iterator it = outputParams.find(name);
            if (it != outputParams.end())
            {
                int index = comboBox->findText(it.value());
                if (index != -1)
                {
                    comboBox->setCurrentIndex(index);
                }
            }
        }
    }
}

void CmdActionModuleEditDialog::onAccept()
{
    if (mCommand)
    {
        QMap<QString, QString> input;
        QMap<QString, QString> output;

        SystemState* system = mCommand->systemState();
        int inCount = system->paramsCount(mModuleID, mCommandID, true);
        int outCount = system->paramsCount(mModuleID, mCommandID, false);

        for (int i = 0; i < inCount; ++i)
        {
            QLabel* label = qobject_cast<QLabel*>(mParams->cellWidget(i, 0));
            QString name;
            if (label)
            {
                name = label->text();
            }

            QComboBox* comboBox = qobject_cast<QComboBox*>(mParams->cellWidget(i, 1));
            if (comboBox)
            {
                input[name] = comboBox->currentText();
            }
        }

        for (int i = inCount; i < (inCount + outCount); ++i)
        {
            QString name;
            QLabel* label = qobject_cast<QLabel*>(mParams->cellWidget(i, 0));
            if (label)
            {
                name = label->text();
            }

            QComboBox* comboBox = qobject_cast<QComboBox*>(mParams->cellWidget(i, 1));
            if (comboBox)
            {
                output[name] = comboBox->currentText();
            }
        }

        // get implicit params (TODO)
        QListWidgetItem* item = mCommands->currentItem();
        int paramsCount = item->data(Qt::UserRole + 1).toInt();

        QList<int> implicitParams;
        for (int j = 0; j < paramsCount; ++j)
        {
            implicitParams.push_back(item->data(Qt::UserRole + j + 2).toInt());
        }

        mCommand->setParams((ModuleCommands::ModuleID)mModuleID, (ModuleCommands::CommandID)mCommandID, input, output, implicitParams);
    }

    accept();
}
