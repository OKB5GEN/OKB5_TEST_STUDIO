#include "Headers/app_settings.h"
#include "Headers/logger/Logger.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>

#include <QMetaEnum>

namespace
{
    static const QString SETTINGS_FILE_NAME = "app_settings.xml";
}

AppSettings::AppSettings()
{
}

AppSettings& AppSettings::instance()
{
    static AppSettings settings;
    return settings;
}

QVariant AppSettings::settingValue(SettingID id) const
{
    return mSettings.value(id, QVariant());
}

QVariant AppSettings::settingValue(const QString& key) const
{
    QMetaEnum metaEnum = QMetaEnum::fromType<AppSettings::SettingID>();
    bool ok = false;
    int id = metaEnum.keyToValue(qPrintable(key), &ok);

    if (!ok)
    {
        LOG_WARNING(QString("Application setting '%1' not found").arg(key));
        return QVariant();
    }

    return mSettings.value(AppSettings::SettingID(id), QVariant());
}

void AppSettings::setSetting(SettingID id, const QVariant& value, bool sendSignal)
{
    mSettings[id] = value;
    if (sendSignal)
    {
        emit settingsChanged();
    }
}

void AppSettings::setSetting(const QString& key, const QVariant& value, bool sendSignal)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<AppSettings::SettingID>();
    bool ok = false;
    int id = metaEnum.keyToValue(qPrintable(key), &ok);

    if (!ok)
    {
        LOG_WARNING(QString("Trying to set application setting '%1' that does not exist").arg(key));
        return;
    }

    setSetting(SettingID(id), value, sendSignal);
}

void AppSettings::load()
{
    mSettings.clear();

    // 1. Try open settings file
    QString fileName = QDir::currentPath() + QString("/") + SETTINGS_FILE_NAME;
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_FATAL(QString("Cannot open application settings file for reading '%1': %2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    // 2. Read settings file
    QXmlStreamReader xml;
    xml.setDevice(&file);

    QMetaEnum metaEnum = QMetaEnum::fromType<AppSettings::SettingID>();

    if (xml.readNextStartElement())
    {
        if (xml.name() == "app_settings" && xml.attributes().value("version") == "1.0")
        {
            while (!xml.atEnd() && !xml.hasError())
            {
                QXmlStreamReader::TokenType token = xml.readNext();

                if (token == QXmlStreamReader::StartElement)
                {
                    QString name = xml.name().toString();

                    if (name == "setting")
                    {
                        QXmlStreamAttributes attributes = xml.attributes();
                        QString key;
                        QString value;

                        if (attributes.hasAttribute("name"))
                        {
                            key = attributes.value("name").toString();
                        }

                        if (attributes.hasAttribute("value"))
                        {
                            value = attributes.value("value").toString();
                        }

                        if (!key.isEmpty() && !value.isEmpty())
                        {
                            bool ok = false;
                            int id = metaEnum.keyToValue(qPrintable(key), &ok);

                            if (ok)
                            {
                                mSettings[SettingID(id)] = value;
                            }
                            else
                            {
                                LOG_WARNING(QString("Unknown setting '%1' found").arg(key));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            xml.raiseError(QObject::tr("The file is not an application settings version 1.0 file."));
        }
    }

    if (!xml.errorString().isEmpty())
    {
        LOG_FATAL(QString("%1\nLine %2, column %3").arg(xml.errorString()).arg(xml.lineNumber()).arg(xml.columnNumber()));
        return;
    }

    loadTexts();

    emit settingsChanged();
}

void AppSettings::save()
{
    QString fileName = QDir::currentPath() + QString("/") + SETTINGS_FILE_NAME;
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot open application settings file for writing '%1': %2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QXmlStreamWriter xml;
    QMetaEnum metaEnum = QMetaEnum::fromType<AppSettings::SettingID>();

    xml.setAutoFormatting(true);
    xml.setDevice(&file);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE app_settings>");
    xml.writeStartElement("app_settings");
    xml.writeAttribute("version", "1.0");

    for (auto it = mSettings.begin(); it != mSettings.end(); ++it)
    {
        xml.writeStartElement("setting");
        xml.writeAttribute("name", metaEnum.key(it.key()));
        xml.writeAttribute("value", it.value().toString());
        xml.writeEndElement();
    }

    xml.writeEndDocument();
}

void AppSettings::loadTexts()
{
    mSettingsNames.clear();
    mSettingsComments.clear();

    mSettingsNames[CYCLOGRAM_WIDGET_SCALE_MIN] = tr("Cyclogram widget min scale");
    mSettingsNames[CYCLOGRAM_WIDGET_SCALE_MAX] = tr("Cyclogram widget max scale");
    mSettingsNames[CYCLOGRAM_WIDGET_SCALE_DEFAULT] = tr("Cyclogram widget default scale");
    mSettingsNames[CYCLOGRAM_WIDGET_SCALE_STEP] = tr("Cyclogram widget scale change step");
    mSettingsNames[CELL_WIDTH] = tr("Cyclogram widget cell width");
    mSettingsNames[CELL_HEIGHT] = tr("Cyclogram widget cell height");
    mSettingsNames[CELLS_PER_ITEM_V] = tr("Command shape width");
    mSettingsNames[CELLS_PER_ITEM_H] = tr("Command shape height");
    mSettingsNames[APP_START_CYCLOGRAM_FILE] = tr("Application start cyclogram file");
    mSettingsNames[APP_FINISH_CYCLOGRAM_FILE] = tr("Application finish cyclogram file");
    mSettingsNames[CYCLOGRAM_START_CYCLOGRAM_FILE] = tr("Pre-action cyclogram");
    mSettingsNames[CYCLOGRAM_FINISH_CYCLOGRAM_FILE] = tr("Post-action cyclogram");
    mSettingsNames[COMMAND_EXECUTION_DELAY] = tr("Command execution delay");
    mSettingsNames[MODULE_RESPONSE_WAIT_TIMEOUT] = tr("Soft module response wait time");
    mSettingsNames[DEFAULT_RESPONSE_WAIT_TIME] = tr("Hard module response wait time");
    mSettingsNames[DEFAULT_SEND_REQUEST_INTERVAL] = tr("Min send request interval");
    mSettingsNames[SOFT_RESET_UPDATE_TIME] = tr("Module up after soft reset check timeout");
    mSettingsNames[MAX_BUP_ALLOWED_VOLTAGE] = tr("Max output voltage");
    mSettingsNames[MAX_BUP_ALLOWED_CURRENT] = tr("Max output current");
    mSettingsNames[MAX_MKO_REPEAT_REQUESTS] = tr("Max requests repeat count");

    mSettingsComments[CYCLOGRAM_WIDGET_SCALE_MIN] = tr("Float. 100% = 1.0");
    mSettingsComments[CYCLOGRAM_WIDGET_SCALE_MAX] = tr("Float. 100% = 1.0");
    mSettingsComments[CYCLOGRAM_WIDGET_SCALE_DEFAULT] = tr("Float. 100% = 1.0");
    mSettingsComments[CYCLOGRAM_WIDGET_SCALE_STEP] = tr("Float. 100% = 1.0");
    mSettingsComments[CELL_WIDTH] = tr("Integer. Pixels count");
    mSettingsComments[CELL_HEIGHT] = tr("Integer. Pixels count");
    mSettingsComments[CELLS_PER_ITEM_V] = tr("Integer. Cells width count");
    mSettingsComments[CELLS_PER_ITEM_H] = tr("Integer. Cells height count");
    mSettingsComments[APP_START_CYCLOGRAM_FILE] = tr("String. Cyclogram file need to be launched at application START (file path relative to APP_FOLDER/cyclograms/");
    mSettingsComments[APP_FINISH_CYCLOGRAM_FILE] = tr("String. Cyclogram file need to be launched at application FINISH (file path relative to APP_FOLDER/cyclograms/");
    mSettingsComments[CYCLOGRAM_START_CYCLOGRAM_FILE] = tr("String. Cyclogram file need to be launched before main cyclogram START (file path relative to APP_FOLDER/cyclograms/");
    mSettingsComments[CYCLOGRAM_FINISH_CYCLOGRAM_FILE] = tr("String. Cyclogram file need to be launched after main cyclogram FINISHED (file path relative to APP_FOLDER/cyclograms/"); //TODO remove this setting
    mSettingsComments[COMMAND_EXECUTION_DELAY] = tr("Integer. Milliseconds. Cyclogram command forced execution delay for visuzalization");
    mSettingsComments[MODULE_RESPONSE_WAIT_TIMEOUT] = tr("Integer. Milliseconds. Waiting for cyclogram module command execution timeout value.");
    mSettingsComments[DEFAULT_RESPONSE_WAIT_TIME] = tr("Integer. Milliseconds. Waiting for COM-port hardware module response timeout");
    mSettingsComments[DEFAULT_SEND_REQUEST_INTERVAL] = tr("Integer. Milliseconds. Minimal interval between two requests sent to same COM-port module");
    mSettingsComments[SOFT_RESET_UPDATE_TIME] = tr("Integer. Milliseconds."); // check COM port module is up after soft reset
    mSettingsComments[MAX_BUP_ALLOWED_VOLTAGE] = tr("Float. Volts. Maximal input voltage for Drive Control Unit");
    mSettingsComments[MAX_BUP_ALLOWED_CURRENT] = tr("Float. Ampers. Maximal input current for Drive Control Unit");
    mSettingsComments[MAX_MKO_REPEAT_REQUESTS] = tr("Interger. Maximum request repeat count if MKO responds with error 'Response not ready'");
}

QString AppSettings::settingName(SettingID id) const
{
    return mSettingsNames.value(id, tr("Setting name not found!"));
}

QString AppSettings::settingComment(SettingID id) const
{
    return mSettingsComments.value(id, tr("Setting comment not found!"));
}
