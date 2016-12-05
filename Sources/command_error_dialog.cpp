#include "Headers/command_error_dialog.h"

#include <QtWidgets>

CommandErrorDialog::CommandErrorDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLabel = new QLabel(this);
    layout->addWidget(mLabel, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Cyclogram Error"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CommandErrorDialog::~CommandErrorDialog()
{

}

void CommandErrorDialog::setText(const QString& text)
{
    mLabel->setText(tr("Command '%1' has errors. Would you like to fix it?").arg(text));
}
