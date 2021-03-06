#include "Headers/gui/cyclogram/dialogs/cmd_state_start_edit_dialog.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/gui/tools/console_text_widget.h"

#include <QtWidgets>

CmdStateStartEditDialog::CmdStateStartEditDialog(QWidget * parent):
    RestorableDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLineEdit = new QLineEdit(this);
    mLineEdit->setText("TEXT");
    layout->addWidget(mLineEdit, 0, 0);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 1, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 0);

    setLayout(layout);
    setWindowTitle(tr("Set Branch Name"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    //setFixedSize(sizeHint());
}

CmdStateStartEditDialog::~CmdStateStartEditDialog()
{

}

void CmdStateStartEditDialog::setCommands(CmdStateStart * command, const QList<Command*>& otherBranches)
{
    mOtherBranches = otherBranches;
    mCommand = command;
    mLineEdit->setText(command->text());
    mConsoleTextWidget->setCommand(mCommand);

    readSettings();
}

void CmdStateStartEditDialog::onAccept()
{
    if (mCommand)
    {
        foreach (Command* cmd, mOtherBranches)
        {
            if (cmd->text() == mLineEdit->text())
            {
                QMessageBox::warning(this, tr("Error"), tr("Branch with such name already exist!\nTry another name"));
                return;
            }
        }

        mCommand->setText(mLineEdit->text());
        mConsoleTextWidget->saveCommand();
    }

    accept();
}
