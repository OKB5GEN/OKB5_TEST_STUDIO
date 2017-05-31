#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QVariant>

class AppSettings
{
public:
    static AppSettings& instance();

    QVariant setting(const QString& key) const;
    void setSetting(const QString& key, const QVariant& value);

    void load();
    void save();

private:
    AppSettings();

    QMap<QString, QVariant> mSettings;
};

#endif // APP_SETTINGS_H
