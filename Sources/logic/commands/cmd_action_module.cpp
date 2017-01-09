#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/system/system_state.h"
#include "Headers/system/modules/module_mko.h"
#include "Headers/system/modules/module_otd.h"
#include "Headers/system/modules/module_power.h"
#include "Headers/system/modules/module_stm.h"
#include "Headers/system/modules/module_tech.h"
//#include "Headers/logic/variable_controller.h"

#include <QTimer>

CmdActionModule::CmdActionModule(QObject* parent):
    CmdAction(DRAKON::ACTION_MODULE, parent),
    mModule(ModuleCommands::POWER_UNIT_BUP),
    mOperation(ModuleCommands::SET_VOLTAGE_AND_CURRENT)
{
    updateText();
}

void CmdActionModule::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(execute()));
    }
    else
    {
        execute();
    }
}

void CmdActionModule::execute()
{
    int TODO; //SEND SIGNALS, NOT DIRECT CALLS!

    switch (mModule)
    {
    case ModuleCommands::POWER_UNIT_BUP:
    case ModuleCommands::POWER_UNIT_PNA:
        {
            ModulePower* module = (mModule == ModuleCommands::POWER_UNIT_BUP) ? mSystemState->modulePowerBUP() : mSystemState->modulePowerPNA();

            switch (mOperation)
            {
            case ModuleCommands::SET_VOLTAGE_AND_CURRENT:
                {
                    module->setVoltageAndCurrent(27); //TODO
                }
                break;
            case ModuleCommands::SET_MAX_VOLTAGE_AND_CURRENT:
                {
                    module->setMaxVoltageAndCurrent(28, 0.5); //TODO
                }
                break;
            case ModuleCommands::SET_POWER_STATE:
                {
                    module->setPowerState(ModuleCommands::POWER_ON); //TODO
                }
                break;
            case ModuleCommands::GET_VOLTAGE_AND_CURRENT:
                {
                    double voltage;
                    double current;
                    uint8_t error;
                    module->getCurVoltageAndCurrent(voltage, current, error); // TODO
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    // read current values from variable controller
/*    for (int i = 0; i < OperandsCount; ++i)
    {
        if (mOperands[i].type == Variable)
        {
            qreal v = mVarCtrl->variable(mOperands[i].variable);
            mOperands[i].value = v;
        }
    }

    // perform operation
    switch (mOperation)
    {
    case Add:
        mOperands[Result].value = mOperands[Operand1].value + mOperands[Operand2].value;
        break;
    case Subtract:
        mOperands[Result].value = mOperands[Operand1].value - mOperands[Operand2].value;
        break;
    case Multiply:
        mOperands[Result].value = mOperands[Operand1].value * mOperands[Operand2].value;
        break;
    case Divide:
        if (mOperands[Operand2].value != 0)
        {
            mOperands[Result].value = mOperands[Operand1].value / mOperands[Operand2].value;
        }
        else
        {
            mErrorText = tr("Division by zero in runtime");
            emit criticalError(this);
        }

        break;
    case Assign:
        mOperands[Result].value = mOperands[Operand1].value;
        break;
    default:
        break;
    }

    // set new variable value to variable controller
    mVarCtrl->setVariable(mOperands[Result].variable, mOperands[Result].value);
    */

    finish();
}

void CmdActionModule::setParams(ModuleCommands::ModuleID module, ModuleCommands::CommandID operation, const QMap<QString, QString>& in, const QMap<QString, QString>& out)
{
    mModule = module;
    mOperation = operation;
    mInputParams = in;
    mOutputParams = out;
    updateText();
}

ModuleCommands::CommandID CmdActionModule::operation() const
{
    return mOperation;
}

ModuleCommands::ModuleID CmdActionModule::module() const
{
    return mModule;
}

const QMap<QString, QString>& CmdActionModule::inputParams() const
{
    return mInputParams;
}

const QMap<QString, QString>& CmdActionModule::outputParams() const
{
    return mOutputParams;
}

void CmdActionModule::updateText()
{
    if (!mSystemState)
    {
        mText = tr("Invalid cmd");
        setErrorStatus(true);
        return;
    }

    mText = "";
    bool isValid = true;

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value().isEmpty())
        {
            isValid = false;
            break;
        }
        else
        {
            if (!mText.isEmpty())
            {
                mText += ",";
            }

            mText += it.value();
        }
    }

    if (!mOutputParams.isEmpty())
    {
        mText += "=";
    }

    if (mInputParams.size() < mSystemState->paramsCount(mModule, mOperation, true))
    {
        isValid = false;
    }

    if (mOutputParams.size() < mSystemState->paramsCount(mModule, mOperation, false))
    {
        isValid = false;
    }

    mText += moduleName();
    mText += ".";
    mText += commandName();
    mText += "(";

    bool isFirstParam = true;

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value().isEmpty())
        {
            isValid = false;
        }
        else
        {
            if (!isFirstParam)
            {
                mText += ",";
            }

            mText += it.value();
        }

        isFirstParam = false;
    }

    mText += ")";

    if ((hasError() && isValid) || (!hasError() && !isValid))
    {
        if (!isValid)
        {
            mText = tr("Invalid cmd");
        }

        setErrorStatus(!isValid);
    }

    emit textChanged(mText);
}

void CmdActionModule::onNameChanged(const QString& newName, const QString& oldName)
{
    QList<QString> values = mInputParams.values();

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value() == oldName)
        {
            mInputParams[it.key()] = newName;
        }
    }

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value() == oldName)
        {
            mOutputParams[it.key()] = newName;
        }
    }

    updateText();
}

void CmdActionModule::onVariableRemoved(const QString& name)
{
    QList<QString> values = mInputParams.values();

    for (QMap<QString, QString>::iterator it = mInputParams.begin(); it != mInputParams.end(); ++it)
    {
        if (it.value() == name)
        {
            mInputParams[it.key()] = "";
        }
    }

    for (QMap<QString, QString>::iterator it = mOutputParams.begin(); it != mOutputParams.end(); ++it)
    {
        if (it.value() == name)
        {
            mOutputParams[it.key()] = "";
        }
    }

    updateText();
}

QString CmdActionModule::moduleName() const
{
    mModule;
    return tr("БП");
}

QString CmdActionModule::commandName() const
{
    mOperation;
    return tr("КМД");
}
