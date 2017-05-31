#include "Headers/app_settings.h"
#include "Headers/logger/Logger.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>

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

QVariant AppSettings::setting(const QString& key) const
{
    return mSettings.value(key, QVariant());
}

void AppSettings::setSetting(const QString& key, const QVariant& value)
{
    if (key.isEmpty())
    {
        LOG_ERROR(QString("Try to set application setting with empty key"));
        return;
    }

    mSettings[key] = value;
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
                            mSettings[key] = value;
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

    xml.setAutoFormatting(true);
    xml.setDevice(&file);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE app_settings>");
    xml.writeStartElement("app_settings");
    xml.writeAttribute("version", "1.0");

    for (auto it = mSettings.begin(); it != mSettings.end(); ++it)
    {
        xml.writeStartElement("setting");
        xml.writeAttribute("name", it.key());
        xml.writeAttribute("value", it.value().toString());
        xml.writeEndElement();
    }

    xml.writeEndDocument();
}
