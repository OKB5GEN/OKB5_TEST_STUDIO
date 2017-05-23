#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include <QVariant>

#include "Headers/logic/commands/cmd_action.h"
#include "Headers/module_commands.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
    CmdActionModule(QObject* parent);

    void run() override;

    void setParams(ModuleCommands::ModuleID module, uint32_t operation, const QMap<QString, QVariant>& in, const QMap<QString, QVariant>& out);

    uint32_t operation() const;
    ModuleCommands::ModuleID module() const;
    const QMap<QString, QVariant>& inputParams() const;
    const QMap<QString, QVariant>& outputParams() const;

    QString moduleName() const;
    static QString moduleName(int moduleId);

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();
    void onCommandFinished(bool success);

private:
    void updateText() override;

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;


    QString commandName() const;

    ModuleCommands::ModuleID mModule;
    uint32_t mOperation;

    QMap<QString, QVariant> mInputParams;
    QMap<QString, QVariant> mOutputParams;
};
#endif // CMD_ACTION_MODULE_H
