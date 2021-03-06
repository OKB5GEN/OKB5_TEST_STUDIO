#ifndef CMD_ACTION_MODULE_EDIT_DIALOG_H
#define CMD_ACTION_MODULE_EDIT_DIALOG_H

#include "Headers/gui/tools/restorable_dialog.h"
#include <QVariant>

class QListWidget;
class QTableWidget;
class QDoubleValidator;

class CmdActionModule;
class ConsoleTextWidget;

class CmdActionModuleEditDialog : public RestorableDialog
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
    void onOutVarChanged(const QString& text);

private:
    void setupUI();
    void addCommand(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& implicitInputParams = QMap<uint32_t, QVariant>());
    void addOKBCommonCommands(uint32_t moduleID);
    void addPowerUnitCommonCommands(uint32_t moduleID);
    bool eventFilter(QObject *obj, QEvent *event) override;

    CmdActionModule* mCommand;

    QListWidget * mModules;
    QListWidget * mSetCommands;
    QListWidget * mGetCommands;
    QTableWidget* mInParams;
    QTableWidget* mOutParams;

    int mModuleID;
    int mCommandID;

    QDoubleValidator* mValidator;

    ConsoleTextWidget* mConsoleTextWidget;
};

#endif // CMD_ACTION_MODULE_EDIT_DIALOG_H
