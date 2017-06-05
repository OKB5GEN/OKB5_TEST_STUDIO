#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QObject>
#include <QVariant>

class AppSettings: public QObject
{
    Q_OBJECT

public:
    enum SettingID
    {
        CYCLOGRAM_WIDGET_SCALE_MIN, // Cyclogram widget scaling params
        CYCLOGRAM_WIDGET_SCALE_MAX, // Cyclogram widget scaling params
        CYCLOGRAM_WIDGET_SCALE_DEFAULT, // Cyclogram widget scaling params
        CYCLOGRAM_WIDGET_SCALE_STEP, // Cyclogram widget scaling params
        CELL_WIDTH, // ShapeItem size and position params
        CELL_HEIGHT, // ShapeItem size and position params
        CELLS_PER_ITEM_V, // ShapeItem size and position params
        CELLS_PER_ITEM_H, // ShapeItem size and position params
        APP_START_CYCLOGRAM_FILE,
        APP_FINISH_CYCLOGRAM_FILE,
        COMMAND_EXECUTION_DELAY, // Cyclogram command "force" command execution delay for visuzalization
        MODULE_RESPONSE_WAIT_TIMEOUT, // Internal timeout waiting response from "program" module (buggy module logic detection)
        DEFAULT_RESPONSE_WAIT_TIME, // COM port module wait for response from real device time
        DEFAULT_SEND_REQUEST_INTERVAL, // COM port module minimum send message interval (in case of queue of messages)
        SOFT_RESET_UPDATE_TIME, // check COM port module is up after soft reset
        MAX_BUP_ALLOWED_VOLTAGE, //Power unit
        MAX_BUP_ALLOWED_CURRENT, //Power unit
        MAX_MKO_REPEAT_REQUESTS, // MKO resensing
    };

    Q_ENUM(SettingID)

    static AppSettings& instance();

    QVariant settingValue(SettingID id) const;
    QVariant settingValue(const QString& key) const;

    QString settingName(SettingID id) const;
    QString settingComment(SettingID id) const;

    void setSetting(const QString& key, const QVariant& value, bool sendSignal = true);
    void setSetting(SettingID id, const QVariant& value, bool sendSignal = true);

    void load();
    void save();

signals:
    void settingsChanged();

private:
    AppSettings();

    void loadTexts();

    QMap<SettingID, QVariant> mSettings;

    QMap<SettingID, QString> mSettingsNames;
    QMap<SettingID, QString> mSettingsComments;
};

#endif // APP_SETTINGS_H
