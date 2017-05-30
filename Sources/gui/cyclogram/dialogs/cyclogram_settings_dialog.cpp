#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"

#include "Headers/logic/cyclogram.h"

#include <QtWidgets>
#include <QVariant>

CyclogramSettingsDialog::CyclogramSettingsDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mCyclogramDescription = new QTextEdit(this);
    mCyclogramDescription->setText(tr("Type cyclogram description here"));

    QGroupBox* descriptionGroupBox = new QGroupBox(tr("Description"), this);
    QVBoxLayout* boxLayout = new QVBoxLayout();
    boxLayout->addWidget(mCyclogramDescription);
    descriptionGroupBox->setLayout(boxLayout);

    layout->addWidget(descriptionGroupBox, 0, 0);

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
    mCyclogramDescription->setPlainText(cyclogram->setting(Cyclogram::SETTING_DESCRIPTION).toString());
}

void CyclogramSettingsDialog::onAccept()
{
    auto cyclogram = mCyclogram.lock();
    cyclogram->setSetting(Cyclogram::SETTING_DESCRIPTION, mCyclogramDescription->toPlainText());

    accept();
}
