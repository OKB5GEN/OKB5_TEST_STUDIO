#ifndef APP_SETTINGS_DIALOG_H
#define APP_SETTINGS_DIALOG_H

#include <QDialog>

class QTextEdit;
class QTableWidget;

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
    QTableWidget* mSettingsTable;
};

#endif // APP_SETTINGS_DIALOG_H
