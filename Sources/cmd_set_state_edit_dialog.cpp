
#include "Headers/cmd_set_state_edit_dialog.h"
#include "Headers/commands/cmd_set_state.h"
#include "Headers/cyclogram.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>

CmdSetStateEditDialog::CmdSetStateEditDialog(QWidget * parent):
    QDialog(parent),
    mCurrentIndex(-1),
    mCommand(Q_NULLPTR)
{
    QGridLayout * layout = new QGridLayout(this);

    mComboBox = new QComboBox(this);
    layout->addWidget(mComboBox, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle("Go To Branch");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setFixedSize(sizeHint());
}

CmdSetStateEditDialog::~CmdSetStateEditDialog()
{

}

void CmdSetStateEditDialog::setCommands(CmdSetState * command, const QList<Command*>& branches)
{
    mCommand = command;
    mComboBox->clear();
    mBranches = branches;

    Command* nextCmd = mCommand->nextCommands()[0];
    mCurrentIndex = -1;
    int i = 0;
    foreach (Command* cmd, branches)
    {
        if (cmd == nextCmd)
        {
            mCurrentIndex = i;
        }

        mComboBox->addItem(cmd->text());
        ++i;
    }

    mComboBox->setCurrentIndex(mCurrentIndex);
}

void CmdSetStateEditDialog::onAccept()
{
    if (mCommand)
    {
        int index = mComboBox->currentIndex();
        if (index != mCurrentIndex)
        {
            mCommand->replaceCommand(mBranches[index]);
            mCommand->setText(mBranches[index]->text());
        }
    }

    accept();
}
