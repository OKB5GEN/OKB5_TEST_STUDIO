#include "Headers/gui/cyclogram/dialogs/cyclogram_settings_dialog.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/app_settings.h"

#include <QtWidgets>
#include <QVariant>

CyclogramSettingsDialog::CyclogramSettingsDialog(QWidget * parent):
    RestorableDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mDefaultNameStr = new QLineEdit(tr("Cyclogram"), this);

    QLabel* nameLabel = new QLabel(tr("Name:"), this);
    nameLabel->setMaximumWidth(100);

    layout->addWidget(nameLabel, 0, 0);
    layout->addWidget(mDefaultNameStr, 0, 1);

    // Cyclogram file
    QGroupBox* filePathBox = new QGroupBox(tr("Cleanup cyclogram"), this);
    QHBoxLayout* fileNameLayout = new QHBoxLayout(filePathBox);
    mFileNameStr = new QLineEdit(filePathBox);
    mFileNameStr->setBackgroundRole(QPalette::Dark);
    QPushButton* browseButton = new QPushButton(filePathBox);
    connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openFile()));

    browseButton->setText(tr("Browse"));
    fileNameLayout->addWidget(new QLabel(tr("File:"), this));
    fileNameLayout->addWidget(mFileNameStr);
    fileNameLayout->addWidget(browseButton);
    filePathBox->setLayout(fileNameLayout);

    layout->addWidget(filePathBox, 1, 0, 1, 2);

    mCyclogramDescription = new QTextEdit(this);
    mCyclogramDescription->setPlaceholderText(tr("Type subprogram description here"));

    QGroupBox* descriptionGroupBox = new QGroupBox(tr("Description"), this);
    QVBoxLayout* boxLayout = new QVBoxLayout();
    boxLayout->addWidget(mCyclogramDescription);
    descriptionGroupBox->setLayout(boxLayout);

    layout->addWidget(descriptionGroupBox, 2, 0, 1, 2);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 3, 0, 1, 2);

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

    mFileNameStr->setText(cyclogram->setting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM).toString());
    mDefaultNameStr->setText(cyclogram->setting(Cyclogram::SETTING_DEFAULT_NAME).toString());

    readSettings();
}

void CyclogramSettingsDialog::onAccept()
{
    auto cyclogram = mCyclogram.lock();

    QString curCyclogramDecr = cyclogram->setting(Cyclogram::SETTING_DESCRIPTION).toString();
    QString curCleanupCyclogramFile = cyclogram->setting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM).toString();
    QString curDefaultName = cyclogram->setting(Cyclogram::SETTING_DEFAULT_NAME).toString();

    QString newCyclogramDescr = mCyclogramDescription->toPlainText();
    QString newCleanupCyclogramFile = mFileNameStr->text();
    QString newDefaultName = mDefaultNameStr->text();

    if (curCyclogramDecr != newCyclogramDescr)
    {
        cyclogram->setSetting(Cyclogram::SETTING_DESCRIPTION, newCyclogramDescr);
    }

    if (curCleanupCyclogramFile != newCleanupCyclogramFile)
    {
        cyclogram->setSetting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM, newCleanupCyclogramFile);
    }

    if (curDefaultName != newDefaultName)
    {
        cyclogram->setSetting(Cyclogram::SETTING_DEFAULT_NAME, newDefaultName);
    }

    accept();
}

void CyclogramSettingsDialog::openFile()
{
    QString path = Cyclogram::defaultStorePath();

    if (!mFileNameStr->text().isEmpty())
    {
        QString currentFileName = path + mFileNameStr->text();
        if (QFileInfo(currentFileName).exists())
        {
            path = QFileInfo(currentFileName).absoluteDir().path();
        }
        else
        {
            LOG_WARNING(QString("Corrupted file reference '%1' detected. Set path to current application directory path").arg(currentFileName));
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open cyclogram file"), path, tr("OKB5 Cyclogram Files (*%1)").arg(AppSettings::extension()));
    if (!fileName.isEmpty())
    {
        // load cyclogram
        QStringList tokens = fileName.split(Cyclogram::defaultStorePath());

        if (tokens.size() != 2)
        {
            LOG_ERROR(QString("Invalid directory. All cyclograms must be stored in %1 or its subfolders").arg(Cyclogram::defaultStorePath()));
            return;
        }

        mFileNameStr->setText(tokens.at(1));
    }
}
