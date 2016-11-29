#include "Headers/cmd_state_start_edit_dialog.h"
#include "Headers/commands/cmd_state_start.h"
#include "Headers/cyclogram.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>

CmdStateStartEditDialog::CmdStateStartEditDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLineEdit = new QLineEdit(this);
    mLineEdit->setText("TEXT");
    layout->addWidget(mLineEdit, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle("Branch Begin");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setFixedSize(sizeHint());
}

CmdStateStartEditDialog::~CmdStateStartEditDialog()
{

}

void CmdStateStartEditDialog::setCommand(CmdStateStart * command)
{
    mCommand = command;
    mLineEdit->setText(command->text());
}

void CmdStateStartEditDialog::onAccept()
{
    if (mCommand)
    {
        mCommand->setText(mLineEdit->text());
    }

    accept();
}
