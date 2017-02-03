#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/logic/variable_controller.h"

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
            //addCommand(tr("Установить ограничение"), ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT);

            //QList<int> params;
            //params.push_back(ModuleCommands::SET_POWER_STATE);
            //params.push_back(ModuleCommands::POWER_ON);
            //addCommand(tr("Включить питание"), params);

            //params.clear();
            //params.push_back(ModuleCommands::SET_POWER_STATE);
            //params.push_back(ModuleCommands::POWER_OFF);
            //addCommand(tr("Выключить питание"), params);

            addCommand(tr("Получить текущее значение"), ModuleCommands::GET_VOLTAGE_AND_CURRENT);
        }
        break;

    case ModuleCommands::MKO: //TODO
        {
            //mCommands->addItem(tr("МКО1"));
            //mCommands->addItem(tr("МКО2"));
            //mCommands->addItem(tr("МКО3"));
        }
        break;

    case ModuleCommands::STM: //TODO
        {
            //addCommand(tr("Получить адрес модуля"), ModuleCommands::GET_MODULE_ADDRESS);
            //addCommand(tr("Получить статусное слово"), ModuleCommands::GET_STATUS_WORD);
            //addCommand(tr("Сброс ошибки"), ModuleCommands::RESET_ERROR);
            //addCommand(tr("Перезагрузить"), ModuleCommands::SOFT_RESET);
            //addCommand(tr("Получить версию прошивки"), ModuleCommands::GET_SOWFTWARE_VER);
            //addCommand(tr("Эхо"), ModuleCommands::ECHO);
            //addCommand(tr("Включить канал СТМ к БП"), ModuleCommands::POWER_CHANNEL_CTRL);
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
            int commandID = mCommands->item(i)->data(Qt::UserRole).toInt();
            if (commandID == mCommand->operation())
            {
                mCommands->setCurrentRow(i);
                break;
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

        mCommand->setParams((ModuleCommands::ModuleID)mModuleID, (ModuleCommands::CommandID)mCommandID, input, output);
    }

    accept();
}
