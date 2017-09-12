#include "Headers/gui/cyclogram/dialogs/cmd_terminator_edit_dialog.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/gui/tools/console_text_widget.h"

#include "Headers/logger/Logger.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>

CmdTerminatorEditDialog::CmdTerminatorEditDialog(QWidget * parent):
    RestorableDialog(parent),
    mCommand(Q_NULLPTR)
{
    QGridLayout * layout = new QGridLayout(this);

//    int TODO; // remove line edit its just for testing
//    QLineEdit* mLineEdit = new QLineEdit(this);
//    mLineEdit->setText("Default value");
//    layout->addWidget(mLineEdit, 0, 0);
//    connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    mConsoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(mConsoleTextWidget, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Edit Terminator Dialog"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CmdTerminatorEditDialog::~CmdTerminatorEditDialog()
{

}

void CmdTerminatorEditDialog::setCommand(CmdTitle* command)
{
    mCommand = command;
    mConsoleTextWidget->setCommand(mCommand);

    readSettings();
}

void CmdTerminatorEditDialog::onAccept()
{
    if (!mCommand)
    {
        LOG_ERROR(QString("No command set for dialog"));
        reject();
        return;
    }

    mConsoleTextWidget->saveCommand();
    accept();
}
