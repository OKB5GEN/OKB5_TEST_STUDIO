#include "Headers/gui/tools/restorable_dialog.h"

#include <QSettings>
#include <QCoreApplication>

RestorableDialog::RestorableDialog(QWidget* parent):
    QDialog(parent)
{
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
}

RestorableDialog::~RestorableDialog()
{

}

void RestorableDialog::readSettings()
{
    QString className = this->metaObject()->className();

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    restoreGeometry(settings.value(className + QString("/geometry")).toByteArray());
}

void RestorableDialog::saveSettings()
{
    QString className = this->metaObject()->className();

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(className + QString("/geometry"), saveGeometry());
}
