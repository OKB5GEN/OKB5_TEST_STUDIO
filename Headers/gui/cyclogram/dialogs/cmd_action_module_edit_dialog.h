#ifndef CMD_ACTION_MODULE_EDIT_DIALOG_H
#define CMD_ACTION_MODULE_EDIT_DIALOG_H

#include <QDialog>
#include <QList>

class QListWidget;
class QTableWidget;
class CmdActionModule;
class QDoubleValidator;

class CmdActionModuleEditDialog : public QDialog
{
    Q_OBJECT

public:
    CmdActionModuleEditDialog(QWidget * parent);
    ~CmdActionModuleEditDialog();

    void setCommand(CmdActionModule* command);

private slots:
    void onAccept();
    void onModuleChanged(int index);
    void onCommandChanged(int index);
    void onCheckBoxStateChanged(int state);

private:
    void setupUI();
    void addCommand(const QString& text, int param);
    void addCommand(const QString& text, const QList<int>& params);

    CmdActionModule* mCommand;

    QListWidget * mModules;
    QListWidget * mCommands;
    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    int mModuleID;
    int mCommandID;

    QDoubleValidator* mValidator;
};

#endif // CMD_ACTION_MODULE_EDIT_DIALOG_H
