#ifndef ABSTRACT_MODULE_H
#define ABSTRACT_MODULE_H

#include <QObject>
#include <QVariant>

#include "Headers/module_commands.h"

struct Transaction
{
    uint32_t moduleID;
    uint32_t commandID;
    QString error; // empty - no error
    QMap<uint32_t, QVariant> inputParams;
    QMap<uint32_t, QVariant> outputParams;

    Transaction()
    {
        clear();
    }

    void clear()
    {
        inputParams.clear();
        outputParams.clear();
        error.clear();
        moduleID = UINT32_MAX;
        commandID = UINT32_MAX;
    }
};

class AbstractModule: public QObject
{
    Q_OBJECT

public:
    AbstractModule(QObject* parent);

    void setEmulator(bool isEmulator);
    bool isEmulator() const;

    void setModuleID(ModuleCommands::ModuleID id);
    ModuleCommands::ModuleID moduleID() const;

    const QString& moduleName() const;
    const QString& errorString() const;

    virtual bool isPhysicallyActive() const;
    bool isLogicallyActive() const;
    bool isAvailable() const;
    void setLogicallyActive(bool active);

public slots:
    virtual void processCommand(const Transaction& request) = 0; // this method must be reimplemented in inherited classes to receive calls from cyclogram commands

protected:
    Transaction mCurrentTransaction;

signals:
    void commandResult(const Transaction& response); // this signal must be sent when module command execution finished (succesful or not)

private:
    ModuleCommands::ModuleID mModuleID;
    QString mModuleName;
    QString mErrorString;
    bool mIsEmulator;
    bool mIsLogicallyActive;
};

#endif // ABSTRACT_MODULE_H
