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
    //mModules->addItem(tr("Блок питания ПНА"));
    //mModules->addItem(tr("МКО"));
    //mModules->addItem(tr("СТМ"));
    //mModules->addItem(tr("ОТД"));
    //mModules->addItem(tr("ТЕХ"));

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
            QListWidgetItem* item1 = new QListWidgetItem();
            item1->setText(tr("Установить текущее значение"));
            item1->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item1);

            QListWidgetItem* item2 = new QListWidgetItem();
            item2->setText(tr("Установить ограничение"));
            item2->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item2);

            QListWidgetItem* item3 = new QListWidgetItem();
            item3->setText(tr("Включить питание"));
            item3->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_POWER_STATE));
            item3->setData(Qt::UserRole + 1, QVariant((int)ModuleCommands::POWER_ON));
            mCommands->addItem(item3);

            QListWidgetItem* item4 = new QListWidgetItem();
            item4->setText(tr("Выключить питание"));
            item4->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_POWER_STATE));
            item4->setData(Qt::UserRole + 1, QVariant((int)ModuleCommands::POWER_OFF));
            mCommands->addItem(item4);

            QListWidgetItem* item5 = new QListWidgetItem();
            item5->setText(tr("Получить текущее значение"));
            item5->setData(Qt::UserRole, QVariant((int)ModuleCommands::GET_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item5);
        }
        break;

    case ModuleCommands::MKO: //TODO
        {
            mCommands->addItem(tr("МКО1"));
            mCommands->addItem(tr("МКО2"));
            mCommands->addItem(tr("МКО3"));
        }
        break;

    case ModuleCommands::STM: //TODO
        {
            mCommands->addItem(tr("СТМ1"));
            mCommands->addItem(tr("СТМ2"));
            mCommands->addItem(tr("СТМ3"));
        }
        break;

    case ModuleCommands::OTD://TODO
        {
            mCommands->addItem(tr("ОТД1"));
            mCommands->addItem(tr("ОТД2"));
            mCommands->addItem(tr("ОТД3"));
        }
        break;

    case ModuleCommands::TECH://TODO
        {
            mCommands->addItem(tr("ТЕХ1"));
            mCommands->addItem(tr("ТЕХ2"));
            mCommands->addItem(tr("ТЕХ3"));
        }
        break;

    default:
        break;
    }

    mCommands->setCurrentRow(0);
}

void CmdActionModuleEditDialog::onCommandChanged(int index)
{
    mParams->clearContents();

    if (index == -1)
    {
        return;
    }

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
