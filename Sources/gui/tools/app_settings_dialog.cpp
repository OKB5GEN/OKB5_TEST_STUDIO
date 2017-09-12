#include "Headers/gui/tools/app_settings_dialog.h"
#include "Headers/app_settings.h"

#include <QtWidgets>
#include <QVariant>
#include <QMetaEnum>

AppSettingsDialog::AppSettingsDialog(QWidget * parent):
    RestorableDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mSettingsTable = new QTableWidget(this);

    QMetaEnum metaEnum = QMetaEnum::fromType<AppSettings::SettingID>();
    int settingsCount = metaEnum.keyCount();
    mSettingsTable->setRowCount(settingsCount);

    QStringList headers;
    headers.append(tr("Parameter"));
    headers.append(tr("Value"));
    headers.append(tr("Comment"));

    mSettingsTable->setColumnCount(headers.size());
    mSettingsTable->setHorizontalHeaderLabels(headers);


    for (int i = 0; i < settingsCount; ++i)
    {
        int column = 0;
        AppSettings::SettingID id = AppSettings::SettingID(i);

        QTableWidgetItem* nameItem = new QTableWidgetItem();
        nameItem->setFlags(nameItem->flags() ^ Qt::ItemIsEditable);
        nameItem->setText(AppSettings::instance().settingName(id));
        mSettingsTable->setItem(i, column, nameItem);

        ++column;

        QTableWidgetItem* valueItem = new QTableWidgetItem();
        valueItem->setText(AppSettings::instance().settingValue(id).toString());
        mSettingsTable->setItem(i, column, valueItem);

        ++column;

        QTableWidgetItem* commentItem = new QTableWidgetItem();
        commentItem->setFlags(commentItem->flags() ^ Qt::ItemIsEditable);
        commentItem->setText(AppSettings::instance().settingComment(id));
        mSettingsTable->setItem(i, column, commentItem);
    }

    layout->addWidget(mSettingsTable, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Application Settings"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mSettingsTable->resizeColumnsToContents();

    resize(800, 400);

    readSettings();
}

AppSettingsDialog::~AppSettingsDialog()
{

}

void AppSettingsDialog::onAccept()
{
    int rowsCount = mSettingsTable->rowCount();

    for (int i = 0; i < rowsCount; ++i)
    {
        AppSettings::SettingID id = AppSettings::SettingID(i);
        QTableWidgetItem* item = mSettingsTable->item(i, 1);
        AppSettings::instance().setSetting(id, item->text(), (i == rowsCount - 1));
    }

    AppSettings::instance().save();
    accept();
}
