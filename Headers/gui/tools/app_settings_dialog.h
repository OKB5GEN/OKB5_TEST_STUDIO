#ifndef APP_SETTINGS_DIALOG_H
#define APP_SETTINGS_DIALOG_H

#include <QDialog>

class QTextEdit;

class AppSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    AppSettingsDialog(QWidget * parent);
    ~AppSettingsDialog();

protected:

private slots:
    void onAccept();

private:
    QTextEdit* mDescription;
};

#endif // APP_SETTINGS_DIALOG_H
