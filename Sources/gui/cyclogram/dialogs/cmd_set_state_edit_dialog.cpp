#include "Headers/gui/cyclogram/dialogs/cmd_set_state_edit_dialog.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/gui/tools/console_text_widget.h"

#include <QtWidgets>

CmdSetStateEditDialog::CmdSetStateEditDialog(QWidget * parent):
    QDialog(parent),
    mCurrentIndex(-1),
    mCommand(Q_NULLPTR)
{
    QGridLayout * layout = new QGridLayout(this);

    mComboBox = new QComboBox(this);
    mComboBox->installEventFilter(this);
    layout->addWidget(mComboBox, 0, 0);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 1, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 0);

    setLayout(layout);
    setWindowTitle(tr("Set Next Branch"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    //setFixedSize(sizeHint());
}

CmdSetStateEditDialog::~CmdSetStateEditDialog()
{

}

void CmdSetStateEditDialog::setCommands(CmdSetState * command, const QList<Command*>& branches)
{
    mCommand = command;
    mComboBox->clear();
    mBranches = branches;

    Command* nextCmd = Q_NULLPTR;

    if (!mCommand->hasError())
    {
        nextCmd = mCommand->nextCommand();
        mCurrentIndex = -1;
    }
    else
    {
        mCurrentIndex = 0;
    }

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
    mConsoleTextWidget->setCommand(mCommand);
}

void CmdSetStateEditDialog::onAccept()
{
    if (mCommand)
    {
        int index = mComboBox->currentIndex();

        if (mCommand->hasError() || index != mCurrentIndex)
        {
            mCommand->replaceCommand(mBranches[index]);
            mCommand->setText(mBranches[index]->text());
        }

        mConsoleTextWidget->saveCommand();
    }

    accept();
}

bool CmdSetStateEditDialog::eventFilter(QObject *obj, QEvent *event)
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
