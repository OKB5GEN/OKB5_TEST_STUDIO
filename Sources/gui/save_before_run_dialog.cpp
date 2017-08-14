#include "Headers/gui/save_before_run_dialog.h"

#include <QtWidgets>

SaveBeforeRunDialog::SaveBeforeRunDialog(QWidget* parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    QLabel* text = new QLabel(tr("Save cyclograms before run?"), this);
    layout->addWidget(text, 0, 0);
    mDoNotAskAgainBox = new QCheckBox(tr("Always save before run"), this);
    layout->addWidget(mDoNotAskAgainBox, 1, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 2, 0);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setWindowTitle(tr("Save cyclograms"));
    setLayout(layout);
}

bool SaveBeforeRunDialog::doNotAskAgain() const
{
    return mDoNotAskAgainBox->isChecked();
}
