#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include <QMap>
#include <QList>

#include "Headers/logic/commands/cmd_action.h"
#include "Headers/module_commands.h"

class CmdActionModule: public CmdAction
{
    Q_OBJECT

public:
    CmdActionModule(QObject* parent);

    void run() override;

    void setParams(ModuleCommands::ModuleID module, ModuleCommands::CommandID operation, const QMap<QString, QString>& in, const QMap<QString, QString>& out, const QList<int>& implicitParams);

    ModuleCommands::CommandID operation() const;
    ModuleCommands::ModuleID module() const;
    const QMap<QString, QString>& inputParams() const;
    const QMap<QString, QString>& outputParams() const;
    const QList<int>& implicitParams() const;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();
    void onCommandFinished(bool success);

private:
    void updateText();

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

    QString moduleName() const;
    QString commandName() const;

    ModuleCommands::ModuleID mModule;
    ModuleCommands::CommandID mOperation;

    QMap<QString, QString> mInputParams;
    QMap<QString, QString> mOutputParams;
    QList<int> mImplicitParams; //TODO possibly use QVariant instead of int

};
#endif // CMD_ACTION_MODULE_H
