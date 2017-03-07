#ifndef ABSTRACT_MODULE_H
#define ABSTRACT_MODULE_H

#include <QObject>
#include "Headers/module_commands.h"

class AbstractModule: public QObject
{
    Q_OBJECT

public:
    enum ModuleState
    {
        NOT_INITIALIZED,      // module state on application start
        INITIALIZING,         // module object created, its initialization started
        INITIALIZED_FAILED,   // module initialization finished with error, module is not ready to process commands (possibly soft reset will help)
        INITIALIZED_OK,       // module initialization succesfully finished, module is raedy to process commands
        SETTING_TO_SAFE_STATE,// module resetting to "cyclogram-applicable" state started
        SAFE_STATE,           // module is ready to process cyclogram commands
        UNSAFE_STATE,         // cyclogram started, some module commands could be executed
        SOFT_RESETTING        // module is in soft reset state
    };

    Q_ENUM(ModuleState)

    AbstractModule(QObject* parent);

    void setModuleID(ModuleCommands::ModuleID id);
    ModuleCommands::ModuleID moduleID() const;

    const QString& moduleName() const;
    ModuleState moduleState() const;

    const QString& errorString() const;

public slots:
    virtual void processCommand(const QMap<uint32_t, QVariant>& request) = 0;
    virtual void setDefaultState() = 0;
    virtual void onApplicationStart() = 0;

protected:
    void setModuleState(ModuleState moduleState, const QString& error = QString(""));

signals:
    void commandResult(const QMap<uint32_t, QVariant>& response);
    void stateChanged(ModuleCommands::ModuleID moduleID, AbstractModule::ModuleState from, AbstractModule::ModuleState to);

private:
    ModuleState mState;
    ModuleCommands::ModuleID mModuleID;
    QString mModuleName;
    QString mErrorString;
};

#endif // ABSTRACT_MODULE_H
