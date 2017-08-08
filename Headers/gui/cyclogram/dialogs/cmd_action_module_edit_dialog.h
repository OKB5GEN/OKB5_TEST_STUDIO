#ifndef CMD_ACTION_MODULE_EDIT_DIALOG_H
#define CMD_ACTION_MODULE_EDIT_DIALOG_H

#include <QDialog>
#include <QVariant>

class QListWidget;
class QTableWidget;
class QDoubleValidator;

class CmdActionModule;
class ConsoleTextWidget;

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
    void onOutVarChanged(const QString& text);

private:
    void setupUI();
    void addCommand(int commandID, const QMap<uint32_t, QVariant>& implicitInputParams = QMap<uint32_t, QVariant>());
    void addOKBCommonCommands();
    void addPowerUnitCommonCommands();
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
