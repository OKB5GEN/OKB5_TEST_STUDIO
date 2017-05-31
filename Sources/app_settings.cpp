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

QVariant AppSettings::setting(SettingID id) const
{
    return mSettings.value(id, QVariant());
}

QVariant AppSettings::setting(const QString& key) const
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
