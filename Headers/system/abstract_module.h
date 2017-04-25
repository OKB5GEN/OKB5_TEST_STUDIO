#ifndef ABSTRACT_MODULE_H
#define ABSTRACT_MODULE_H

#include <QObject>
#include <QVariant>

#include "Headers/module_commands.h"

struct Transaction
{
    uint32_t moduleID;
    uint32_t commandID;
    uint32_t errorCode; // 0 - no error
    QMap<uint32_t, QVariant> inputParams;
    QMap<uint32_t, QVariant> outputParams;
    QList<int> implicitInputParams; //TODO remove implicit params, move them to inputParams

    Transaction()
    {
        clear();
    }

    void clear()
    {
        inputParams.clear();
        outputParams.clear();
        implicitInputParams.clear();
        errorCode = 0; // no error by default
        moduleID = UINT32_MAX;
        commandID = UINT32_MAX;
    }
};

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

    void setEmulator(bool isEmulator);
    bool isEmulator() const;

    void setModuleID(ModuleCommands::ModuleID id);
    ModuleCommands::ModuleID moduleID() const;

    const QString& moduleName() const;
    ModuleState moduleState() const;

    const QString& errorString() const;

public slots:
    virtual void processCommand(const Transaction& request) = 0;
    virtual void setDefaultState() = 0;
    virtual void onApplicationStart() = 0;

protected:
    void setModuleState(ModuleState moduleState, const QString& error = QString(""));

    Transaction mCurrentTransaction;

signals:
    void commandResult(const Transaction& response);
    void stateChanged(ModuleCommands::ModuleID moduleID, AbstractModule::ModuleState from, AbstractModule::ModuleState to);

private:
    ModuleState mState;
    ModuleCommands::ModuleID mModuleID;
    QString mModuleName;
    QString mErrorString;
    bool mIsEmulator;
};

#endif // ABSTRACT_MODULE_H
