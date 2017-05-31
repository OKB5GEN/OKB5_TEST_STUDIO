#include "Headers/gui/tools/app_settings_dialog.h"

#include <QtWidgets>
#include <QVariant>

AppSettingsDialog::AppSettingsDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mDescription = new QTextEdit(this);
    //mDescription->setText(tr("Type cyclogram description here"));

    QGroupBox* descriptionGroupBox = new QGroupBox(tr("Description"), this);
    QVBoxLayout* boxLayout = new QVBoxLayout();
    boxLayout->addWidget(mDescription);
    descriptionGroupBox->setLayout(boxLayout);

    layout->addWidget(descriptionGroupBox, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Application Settings"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AppSettingsDialog::~AppSettingsDialog()
{

}

void AppSettingsDialog::onAccept()
{
    accept();
}
