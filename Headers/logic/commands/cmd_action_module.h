#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include <QMap>

#include "Headers/logic/commands/cmd_action.h"
#include "Headers/module_commands.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
    CmdActionModule(QObject* parent);

    void run() override;

    void setParams(ModuleCommands::ModuleID module, ModuleCommands::CommandID operation, const QMap<QString, QString>& in, const QMap<QString, QString>& out);

    ModuleCommands::CommandID operation() const;
    ModuleCommands::ModuleID module() const;
    const QMap<QString, QString>& inputParams() const;
    const QMap<QString, QString>& outputParams() const;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();

private:
    void updateText();
    QString moduleName() const;
    QString commandName() const;

    ModuleCommands::ModuleID mModule;
    ModuleCommands::CommandID mOperation;

    QMap<QString, QString> mInputParams;
    QMap<QString, QString> mOutputParams;
};
#endif // CMD_ACTION_MODULE_H
