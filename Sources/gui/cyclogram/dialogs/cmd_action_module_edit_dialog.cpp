#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
//#include "Headers/logic/variable_controller.h"

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
    mModules->addItem(tr("БП БУП"));
    mModules->addItem(tr("БП ПНА"));
    mModules->addItem(tr("МКО"));
    mModules->addItem(tr("СТМ"));
    mModules->addItem(tr("ОТД"));
    mModules->addItem(tr("ТЕХ"));

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
            item1->setText(tr("УСТ.НАПР"));
            item1->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item1);

            QListWidgetItem* item2 = new QListWidgetItem();
            item2->setText(tr("УСТ.МАКС.НАПР"));
            item2->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item2);

            QListWidgetItem* item3 = new QListWidgetItem();
            item3->setText(tr("УСТ.СОСТ"));
            item3->setData(Qt::UserRole, QVariant((int)ModuleCommands::SET_POWER_STATE));
            mCommands->addItem(item3);

            QListWidgetItem* item4 = new QListWidgetItem();
            item4->setText(tr("ПОЛ.НАПР"));
            item4->setData(Qt::UserRole, QVariant((int)ModuleCommands::GET_VOLTAGE_AND_CURRENT));
            mCommands->addItem(item4);
        }
        break;

    case ModuleCommands::MKO:
        {
            mCommands->addItem(tr("МКО1"));
            mCommands->addItem(tr("МКО2"));
            mCommands->addItem(tr("МКО3"));
        }
        break;

    case ModuleCommands::STM:
        {
            mCommands->addItem(tr("СТМ1"));
            mCommands->addItem(tr("СТМ2"));
            mCommands->addItem(tr("СТМ3"));
        }
        break;

    case ModuleCommands::OTD:
        {
            mCommands->addItem(tr("ОТД1"));
            mCommands->addItem(tr("ОТД2"));
            mCommands->addItem(tr("ОТД3"));
        }
        break;

    case ModuleCommands::TECH:
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
    int count = system->paramsCount(mModuleID, mCommandID);
    mParams->setRowCount(count);

    for (int i = 0; i < count; ++i)
    {
        QString name = system->paramName(mModuleID, mCommandID, i);
        QLabel* text = new QLabel(mParams);
        text->setText(name);
        mParams->setCellWidget(i, 0, text);

        //SystemState::ParamType type = system->paramType(mModuleID, mCommandID, i);

        QLineEdit* lineEdit = new QLineEdit(mParams);
        lineEdit->setValidator(mValidator);
        mParams->setCellWidget(i, 1, lineEdit);
    }
}

void CmdActionModuleEditDialog::onAccept()
{
    if (mCommand)
    {
        // TODO : set GUI data to command
    }

    accept();
}
