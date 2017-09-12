#include "Headers/gui/cyclogram/dialogs/cmd_parallel_process_edit_dialog.h"

#include "Headers/gui/tools/console_text_widget.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>

CmdParallelProcessEditDialog::CmdParallelProcessEditDialog(QWidget * parent):
    RestorableDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    int TODO; // remove line edit its just for testing
    QLineEdit* mLineEdit = new QLineEdit(this);
    mLineEdit->setText("Default value");
    layout->addWidget(mLineEdit, 0, 0);

    ConsoleTextWidget* consoleTextWidget = new ConsoleTextWidget(this);
    layout->addWidget(consoleTextWidget, 1, 0);
    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 0);

    setLayout(layout);
    setWindowTitle(tr("Parallel Process Edit Dialog"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CmdParallelProcessEditDialog::~CmdParallelProcessEditDialog()
{

}

void CmdParallelProcessEditDialog::setCommand(Command* command)
{
    int TODO; // create GUI for command params editing

    readSettings();
}
