#include "Headers/cyclogram_end_dialog.h"

#include <QtWidgets>

CyclogramEndDialog::CyclogramEndDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setText(tr("ПОТРАЧЕНО"));
    layout->addWidget(mLabel, 0, 0);

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Cyclogram finished"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CyclogramEndDialog::~CyclogramEndDialog()
{

}

void CyclogramEndDialog::setText(const QString& text)
{
    mLabel->setText(text);
}
