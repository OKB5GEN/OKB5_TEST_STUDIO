#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"

#include "Headers/logic/cyclogram.h"

#include <QtWidgets>

CyclogramSettingsDialog::CyclogramSettingsDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mCyclogramDescription = new QTextEdit(this);
    mCyclogramDescription->setText(tr("Type cyclogram description here"));
    layout->addWidget(mCyclogramDescription, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Cyclogram Settings"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CyclogramSettingsDialog::~CyclogramSettingsDialog()
{

}

void CyclogramSettingsDialog::setCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    mCyclogram = cyclogram;

    int TODO; // load GUI state from cyclogram
}

void CyclogramSettingsDialog::onAccept()
{
    int TODO; // save settings to cyclogram file

    accept();
}
