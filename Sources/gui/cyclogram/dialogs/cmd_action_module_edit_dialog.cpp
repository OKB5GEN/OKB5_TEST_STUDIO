#include <QtWidgets>

#include "Headers/gui/cyclogram/dialogs/cmd_action_module_edit_dialog.h"
#include "Headers/logic/commands/cmd_action_module.h"
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

    mModules->setCurrentRow(0);

    setLayout(layout);
}

void CmdActionModuleEditDialog::setCommand(CmdActionModule* command)
{
    mCommand = command;

    if (mCommand)
    {
    }
}

void CmdActionModuleEditDialog::onModuleChanged(int index)
{
    mModuleID = index;
    mCommands->clear();

    // module changed -> update command list for this module
    switch (index)
    {
    case CmdActionModule::POWER_UNIT_BUP:
        {
            mCommands->addItem(tr("1УСТ.НАПР"));
            mCommands->addItem(tr("1УСТ.МАКС.НАПР"));
            mCommands->addItem(tr("1УСТ.СОСТ"));
            mCommands->addItem(tr("1ПОЛ.НАПР"));
        }
        break;

    case CmdActionModule::POWER_UNIT_PNA:
        {
            mCommands->addItem(tr("2УСТ.НАПР"));
            mCommands->addItem(tr("2УСТ.МАКС.НАПР"));
            mCommands->addItem(tr("2УСТ.СОСТ"));
            mCommands->addItem(tr("2ПОЛ.НАПР"));
        }
        break;

    case CmdActionModule::MKO:
        {
            mCommands->addItem(tr("МКО1"));
            mCommands->addItem(tr("МКО2"));
            mCommands->addItem(tr("МКО3"));
        }
        break;

    case CmdActionModule::STM:
        {
            mCommands->addItem(tr("СТМ1"));
            mCommands->addItem(tr("СТМ2"));
            mCommands->addItem(tr("СТМ3"));
        }
        break;

    case CmdActionModule::OTD:
        {
            mCommands->addItem(tr("ОТД1"));
            mCommands->addItem(tr("ОТД2"));
            mCommands->addItem(tr("ОТД3"));
        }
        break;

    case CmdActionModule::TECH:
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
    mCommandID = index;
    mParams->clearContents();
}

void CmdActionModuleEditDialog::onAccept()
{
    if (mCommand)
    {
        // TODO : set GUI data to command
    }

    accept();
}

