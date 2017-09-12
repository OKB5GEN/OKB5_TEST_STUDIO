#include "Headers/gui/cyclogram/dialogs/cmd_output_edit_dialog.h"

#include "Headers/gui/tools/console_text_widget.h"

#include <QDialogButtonBox>
#include <QGridLayout>

CmdOutputEditDialog::CmdOutputEditDialog(QWidget * parent):
    RestorableDialog(parent),
    mCommand(Q_NULLPTR)
{
    QGridLayout * layout = new QGridLayout(this);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Message Command Edit Dialog"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CmdOutputEditDialog::~CmdOutputEditDialog()
{

}

void CmdOutputEditDialog::setCommand(Command* command)
{
    mCommand = command;
    mConsoleTextWidget->setCommand(mCommand);

    readSettings();
}

void CmdOutputEditDialog::onAccept()
{
    if (mCommand)
    {
        mConsoleTextWidget->saveCommand();
    }

    accept();
}
