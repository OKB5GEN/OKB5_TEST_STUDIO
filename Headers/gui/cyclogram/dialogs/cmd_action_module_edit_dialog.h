#ifndef CMD_ACTION_MODULE_EDIT_DIALOG_H
#define CMD_ACTION_MODULE_EDIT_DIALOG_H

#include <QDialog>

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

private:
    void setupUI();

    CmdActionModule* mCommand;

    QListWidget * mModules;
    QListWidget * mCommands;
    QTableWidget* mParams;

    int mModuleID;
    int mCommandID;

    QDoubleValidator* mValidator;
};

#endif // CMD_ACTION_MODULE_EDIT_DIALOG_H
