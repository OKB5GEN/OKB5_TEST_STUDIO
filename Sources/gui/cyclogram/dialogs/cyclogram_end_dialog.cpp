#include "Headers/gui/cyclogram/dialogs/cyclogram_end_dialog.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

namespace
{
    static const QString SETTING_LAST_SAVE_FILE_DIR = "LastSaveFileDir"; //TODO remove duplication
}

CyclogramEndDialog::CyclogramEndDialog(QWidget * parent):
    QDialog(parent),
    mCyclogram(Q_NULLPTR)
{
    QGridLayout * layout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setText(tr("ПОТРАЧЕНО"));
    layout->addWidget(mLabel, 0, 0);

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    QPushButton* saveBtn = buttonBox->addButton(tr("Save report as..."), QDialogButtonBox::ActionRole);

    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(saveReportAs()));

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

void CyclogramEndDialog::saveReportAs()
{
    if (!mCyclogram)
    {
        LOG_ERROR(QString("Cyclogram not set"));
        return;
    }

    // try read last file save path
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString path = settings.value(SETTING_LAST_SAVE_FILE_DIR).toString();
    if (path.isEmpty())
    {
        path = QDir::currentPath();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save report file"), path, tr("Excel file (*.xlsx)"));

    if (fileName.isEmpty())
    {
        return;
    }

    mCyclogram->variableController()->saveReport(fileName);
}

void CyclogramEndDialog::setCyclogram(Cyclogram* cyclogram)
{
    mCyclogram = cyclogram;
}
