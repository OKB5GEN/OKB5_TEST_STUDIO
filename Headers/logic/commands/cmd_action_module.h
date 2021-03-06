#ifndef CMD_ACTION_MODULE_H
#define CMD_ACTION_MODULE_H

#include <QVariant>

#include "Headers/logic/command.h"
#include "Headers/module_commands.h"

class CmdActionModule: public Command
{
    Q_OBJECT

public:
    CmdActionModule(QObject* parent);

    void run() override;

    void setParams(ModuleCommands::ModuleID module, uint32_t operation, const QMap<uint32_t, QVariant>& in, const QMap<uint32_t, QVariant>& out);

    uint32_t operation() const;
    ModuleCommands::ModuleID module() const;
    const QMap<uint32_t, QVariant>& inputParams() const;
    const QMap<uint32_t, QVariant>& outputParams() const;

    QString moduleName(bool isFullName) const;
    static QString moduleName(int moduleId, bool isFullName);

    static QString commandName(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& inputParams = QMap<uint32_t, QVariant>());
    static QString paramsText(uint32_t moduleID, uint32_t commandID, const QMap<uint32_t, QVariant>& inputParams = QMap<uint32_t, QVariant>());

    bool isVariableUsed(const QString& name) const override;

protected:
    bool loadFromImpl(Command* other) override;

private slots:
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;
    void execute();
    void onCommandFinished(bool success);

private:
    void updateText() override;
    QString moduleNameImpl() const;

    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion) override;

    ModuleCommands::ModuleID mModule;
    uint32_t mOperation;

    QMap<uint32_t, QVariant> mInputParams;
    QMap<uint32_t, QVariant> mOutputParams;
};
#endif // CMD_ACTION_MODULE_H
