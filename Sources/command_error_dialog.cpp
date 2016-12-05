#include "Headers/command_error_dialog.h"

#include <QtWidgets>

CommandErrorDialog::CommandErrorDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLabel = new QLabel(this);
    layout->addWidget(mLabel, 0, 0);

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(tr("Yes"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("No"), QDialogButtonBox::RejectRole);

    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle("Error");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CommandErrorDialog::~CommandErrorDialog()
{

}

void CommandErrorDialog::setText(const QString& text)
{
    mLabel->setText(tr("Command %1 has errors. Would you like to fix it?").arg(text));
}
