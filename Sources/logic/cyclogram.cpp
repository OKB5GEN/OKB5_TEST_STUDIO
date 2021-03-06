#include <QTime>
#include <QTimer>
#include <QMetaEnum>
#include <QDir>

#include "Headers/logic/cyclogram.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/logic/commands/cmd_parallel_process.h"
#include "Headers/logic/commands/cmd_output.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/commands/cmd_sub_program.h"
#include "Headers/logic/commands/cmd_select_state.h"
#include "Headers/logic/commands/cmd_cycle.h"
#include "Headers/logic/commands/cmd_condition.h"

#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/app_settings.h"
#include "Headers/system/system_state.h"
#include "Headers/file_reader.h"


const QString Cyclogram::SETTING_DESCRIPTION = "description";
const QString Cyclogram::SETTING_CLEANUP_CYCLOGRAM = "cleanup_cyclogram";
const QString Cyclogram::SETTING_DEFAULT_NAME = "default_name";

namespace
{
    static const int COMMAND_RUN_INTERVAL = 10; // msec (TODO maybe try to reduce to 1 ms?)

    void LogCmd(Command* cmd, const QString& state)
    {
        static QMetaEnum metaEnum;
        if (metaEnum.keyCount() == 0)
        {
            metaEnum = QMetaEnum::fromType<DRAKON::IconType>();
        }

        QString text(metaEnum.valueToKey(cmd->type()));
        text += ":";
        text += cmd->text();

        LOG_INFO(QString("Command '%1' is %2").arg(text).arg(state));
    }
}

Cyclogram::Cyclogram(QObject * parent):
    QObject(parent),
    mFirst(Q_NULLPTR),
    mLast(Q_NULLPTR),
    mCurrent(Q_NULLPTR),
    mState(IDLE),
    mVarController(Q_NULLPTR),
    mSystemState(Q_NULLPTR),
    mIsMainCyclogram(false),
    mModified(false)
{
    mVarController = new VariableController(this);

    QString cleanupCyclogram = setting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM).toString();
    if (cleanupCyclogram.isEmpty())
    {
        setSetting(Cyclogram::SETTING_CLEANUP_CYCLOGRAM, QString("on_cyclogram_finish%1").arg(AppSettings::extension()), false); // set cleanup cyclogram by default
    }

    connect(mVarController, SIGNAL(variableAdded(const QString&, qreal)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(variableRemoved(const QString&)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(initialValueChanged(const QString&, qreal)), this, SLOT(variableInitialValueChanged(const QString&, qreal)));
    connect(mVarController, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(variableCurrentValueChanged(const QString&, qreal)));
    connect(mVarController, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(descriptionChanged(const QString&, const QString&)), this, SLOT(variablesChanged()));

    connect(&AppSettings::instance(), SIGNAL(settingsChanged()), this, SLOT(onAppSettingsChanged()));
}

void Cyclogram::createDefault()
{
    clear();

    // empty silhouette
    Command* begin = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(begin);
        tmp->setTitleType(CmdTitle::BEGIN);
    }

    Command* branch1 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch1);
        tmp->setText(tr("Start"));
    }

    Command* toBranch2 = createCommand(DRAKON::GO_TO_BRANCH);
    {

    }

    Command* branch2 = createCommand(DRAKON::BRANCH_BEGIN);
    {
        CmdStateStart* tmp = qobject_cast<CmdStateStart*>(branch2);
        tmp->setText(tr("End"));
    }

    Command* end = createCommand(DRAKON::TERMINATOR);
    {
        CmdTitle* tmp = qobject_cast<CmdTitle*>(end);
        tmp->setTitleType(CmdTitle::END);
    }

    begin->replaceCommand(branch1);
    branch1->replaceCommand(toBranch2);
    toBranch2->replaceCommand(branch2);
    branch2->replaceCommand(end);

    mFirst = begin;
    mLast = end;

    mCurrent = Q_NULLPTR;

    emit modified();
}

void Cyclogram::run()
{
    LOG_INFO(QString("=================================="));
    if (mIsMainCyclogram)
    {
        LOG_INFO(QString("Cyclogram started"));
    }
    else
    {
        LOG_INFO(QString("Subprogram started"));
    }

    LOG_INFO(QString("=================================="));

    if ((mState == IDLE || mState == PENDING_FOR_START) && mFirst != Q_NULLPTR)
    {
        mCurrent = mFirst;

        if (mIsMainCyclogram)
        {
            mVarController->restart(); // set current values to initial values if it is main cyclogram
            //mSystemState->onCyclogramStart();

            // clear all subprograms variables data
            for (auto it = mCommands.begin(); it != mCommands.end(); ++it)
            {
                if ((*it)->type() == DRAKON::SUBPROGRAM)
                {
                    CmdSubProgram* subprogram = qobject_cast<CmdSubProgram*>(*it);
                    subprogram->restart();
                }
            }
        }
        else
        {
            // subprogram case: current values for variable controller will be set by subprogram command

            // start new data timeline
            mVarController->clearDataTimeline(); // TODO проверить, что данные не профакиваются (начальные значения переменных, задаются извне)
        }

        setState(RUNNING);
        runCurrentCommand();
    }
    else
    {
        LOG_ERROR(QString("Icorrect cyclogram state"));
    }
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    emit commandFinished(mCurrent);

    mCurrent->setActive(false);
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    //LogCmd(mCurrent, "finished");

    if (cmd != Q_NULLPTR)
    {
        mCurrent = cmd;

        if (mState == RUNNING)
        {
            QTimer::singleShot(COMMAND_RUN_INTERVAL, this, SLOT(runCurrentCommand())); // to avoid direct slot after signal calling run next command after short timeout
        }
    }
    else
    {
        LOG_INFO(QString("=================================="));
        if (mIsMainCyclogram)
        {
            LOG_INFO(QString("Main cyclogram finished"));
        }
        else
        {
            LOG_INFO(QString("Subprogram finished"));
        }

        LOG_INFO(QString("=================================="));

        stop();
        emit finished("");
    }
}

void Cyclogram::onCriticalError(Command* cmd)
{
    emit commandFinished(mCurrent);

    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    LogCmd(mCurrent, "critical error: " + cmd->errorDesc());

    LOG_ERROR(QString("=================================="));
    if (mIsMainCyclogram)
    {
        LOG_ERROR(QString("Cyclogram stopped due to critical runtime error"));
    }
    else
    {
        LOG_ERROR(QString("Subprogram stopped due to critical runtime error"));
    }

    LOG_ERROR(QString("=================================="));

    for (auto it = mVarController->variablesData().begin(); it != mVarController->variablesData().end(); ++it)
    {
        LOG_ERROR(QString("Variable '%1' current value is %2").arg(it.key()).arg(it.value().currentValue));
    }

    stop();
    emit finished(tr("Critical error occured: ") + cmd->errorDesc());
}

void Cyclogram::runCurrentCommand()
{
    if (!mCurrent)
    {
        return;
    }

    connect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    connect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));

    LogCmd(mCurrent, "started");

    emit commandStarted(mCurrent);
    mCurrent->setActive(true);
    mCurrent->run();
}

void Cyclogram::stop()
{
#ifdef ENABLE_CYCLOGRAM_PAUSE
    if (mState == RUNNING || mState == PAUSED)
#else
    if (mState == RUNNING)
#endif
    {
        mCurrent->setActive(false);
        disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
        mCurrent->stop();

        mSystemState->stop();
    }

    mCurrent = mFirst;
    setState(mIsMainCyclogram ? IDLE : PENDING_FOR_START);
}

#ifdef ENABLE_CYCLOGRAM_PAUSE
void Cyclogram::pause()
{
    if (mState == RUNNING)
    {
        mCurrent->pause();
        setState(PAUSED);
    }
}

void Cyclogram::resume()
{
    if (mState == PAUSED)
    {
        mCurrent->resume();
        setState(RUNNING);
    }
}
#endif

Command* Cyclogram::first() const
{
    return mFirst;
}

Command* Cyclogram::last() const
{
    return mLast;
}

Command* Cyclogram::current() const
{
    return mCurrent;
}

Cyclogram::State Cyclogram::state() const
{
    return mState;
}

void Cyclogram::clear()
{
    foreach (Command* cmd, mCommands)
    {
        emit deleted(cmd);
        deleteCommandImpl(cmd, true);
    }

    mFirst = Q_NULLPTR;
    mCurrent = Q_NULLPTR;
    mLast = Q_NULLPTR;
    mCommands.clear();
    mVarController->clear();
}

void Cyclogram::deleteCommandTree(Command* cmd, bool silent, bool isBranchDeletion)
{
    emit deleted(cmd);

    // put command out of commands list
    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            mCommands.takeAt(i);
            break;
        }
    }

    // recursively delete all child commands
    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        Command* command = cmd->nextCommands()[i];
        if (!command)
        {
            continue;
        }

        if (!isBranchDeletion) // real branch deletion
        {
            deleteCommandTree(command, silent, isBranchDeletion);
        }
        else if (cmd->type() != DRAKON::GO_TO_BRANCH) // branch clipboard copy deletion
        {
            deleteCommandTree(command, silent, isBranchDeletion); // delete further command tree only if it is not a GO_TO_BRANCH command
        }
    }

    deleteCommandImpl(cmd, silent);
}

void Cyclogram::deleteBranch(Command *cmd)
{
    deleteCommandTree(cmd, true, true);
}

void Cyclogram::deleteCommand(Command* cmd, bool recursive /*= false*/)
{
    if (recursive)
    {
        deleteCommandTree(cmd, false, false);
        return;
    }

    emit deleted(cmd);

    if (cmd->nextCommand())
    {
        // update other commands links if the refer to command being deleted
        foreach (Command* command, mCommands)
        {
            for (int i = 0, sz = command->nextCommands().size(); i < sz; ++i)
            {
                Command* cur = command->nextCommands()[i];
                if (cur && cur == cmd)
                {
                    command->replaceCommand(cmd->nextCommand(), cmd);
                }
            }
        }
    }

    // remove command from commands list
    for (int i = 0, sz = mCommands.size(); i < sz; ++i)
    {
        if (mCommands[i] == cmd)
        {
            Command* tmp = mCommands.takeAt(i);
            deleteCommandImpl(tmp, false);
            break;
        }
    }
}

void Cyclogram::deleteCommandImpl(Command* cmd, bool silent)
{
    disconnect(cmd, SIGNAL(dataChanged(const QString&)), this, SLOT(onCommandTextChanged(QString)));
    cmd->deleteLater();
    setModified(true, !silent, false); // modified on command deletion
}

Command* Cyclogram::createCommand(DRAKON::IconType type)
{
    Command* cmd = Q_NULLPTR;

    switch (type)
    {
    case DRAKON::TERMINATOR:
        {
            cmd = new CmdTitle(this);
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            cmd = new CmdStateStart(this);
        }
        break;
    case DRAKON::GO_TO_BRANCH:
        {
            cmd = new CmdSetState(this);
        }
        break;
    case DRAKON::DELAY:
        {
            cmd = new CmdDelay(this);
        }
        break;
    case DRAKON::SUBPROGRAM:
        {
            CmdSubProgram* tmp = new CmdSubProgram(this);
            connect(tmp, SIGNAL(commandStarted(Command*)), this, SIGNAL(commandStarted(Command*)));
            connect(tmp, SIGNAL(commandFinished(Command*)), this, SIGNAL(commandFinished(Command*)));
            tmp->setLoaded(true);
            cmd = tmp;
        }
        break;
    case DRAKON::ACTION_MATH:
        {
            cmd = new CmdActionMath(this);
        }
        break;

    case DRAKON::CONDITION:
        {
            cmd = new CmdCondition(this);
        }
        break;
    case DRAKON::SELECT_STATE:
        {
            cmd = new CmdSelectState(this);
        }
        break;
    case DRAKON::CYCLE:
        {
            cmd = new CmdCycle(this);
        }
        break;
    case DRAKON::ACTION_MODULE:
        {
            cmd = new CmdActionModule(this);
        }
        break;
    case DRAKON::OUTPUT:
        {
            cmd = new CmdOutput(this);
        }
        break;
    case DRAKON::PARALLEL_PROCESS:
        {
            cmd = new CmdParallelProcess(this);
        }
        break;

    default:
        {
            LOG_ERROR(QString("Command %1 not implemented. 0 ms Delay command created").arg(int(type)));
            cmd = new CmdDelay(this); // TODO create "doing nothing" command
        }
        break;
    }

    if (cmd)
    {
        cmd->setVariableController(mVarController);
        cmd->setSystemState(mSystemState);

        mCommands.push_back(cmd);
        connect(cmd, SIGNAL(dataChanged(const QString&)), this, SLOT(onCommandTextChanged(QString)));
        setModified(true, true, false); // modified on command adding
    }

    return cmd;
}

const QList<Command*>& Cyclogram::commands() const
{
    return mCommands;
}

void Cyclogram::setState(State state)
{
    static QMetaEnum metaEnum;
    if (metaEnum.keyCount() == 0)
    {
        metaEnum = QMetaEnum::fromType<Cyclogram::State>();
    }

    QString text(metaEnum.valueToKey(state));

    //LOG_DEBUG(QString("Cyclogram state changed to %1").arg(text));

    if (state == IDLE)
    {
        foreach (Command* command, mCommands)
        {
            if (command->type() == DRAKON::SUBPROGRAM)
            {
                CmdSubProgram* subprogram = qobject_cast<CmdSubProgram*>(command);
                subprogram->cyclogram()->setState(IDLE);
            }
        }
    }

    mState = state;
    emit stateChanged(mState);
}

Command* Cyclogram::validate() const
{
    foreach (Command* command, mCommands)
    {
        if (command->type() == DRAKON::SUBPROGRAM)
        {
            CmdSubProgram* subprogram = qobject_cast<CmdSubProgram*>(command);

            Command* commandWithError = subprogram->cyclogram()->validate();
            if (commandWithError)
            {
                return commandWithError;
            }
        }

        if (command->hasError())
        {
            return command;
        }
    }

    return Q_NULLPTR;
}

VariableController* Cyclogram::variableController() const
{
    return mVarController;
}

void Cyclogram::setSystemState(SystemState* state)
{
    mSystemState = state;

    foreach (Command* cmd, mCommands)
    {
        cmd->setSystemState(state);
    }
}

SystemState* Cyclogram::systemState() const
{
    return mSystemState;
}

void Cyclogram::getBranches(QList<Command*>& branches) const
{
    if (!mFirst || !mLast)
    {
        return;
    }

    // find first and last branches
    Command* firstBranch = mFirst->nextCommand();
    Command* lastBranch = Q_NULLPTR;

    // get other branches
    foreach (Command* it, mCommands)
    {
        if (it->type() == DRAKON::BRANCH_BEGIN && it != firstBranch)
        {
            if (!lastBranch && isCyclogramEndBranch(it))
            {
                lastBranch = it;
            }

            if (it != lastBranch)
            {
                branches.push_back(it);
            }
        }
    }

    branches.push_front(firstBranch);
    branches.push_back(lastBranch);
}

bool Cyclogram::isCyclogramEndBranch(Command* cmd)
{
    if (!cmd)
    {
        return false;
    }

    if (cmd->type() == DRAKON::TERMINATOR)
    {
        return true;
    }
    else if (cmd->type() == DRAKON::GO_TO_BRANCH)
    {
        return false; // do not search further
    }

    for (int i = 0, sz = cmd->nextCommands().size(); i < sz; ++i)
    {
        if (cmd->nextCommands()[i] && isCyclogramEndBranch(cmd->nextCommands()[i]))
        {
            return true;
        }
    }

    return false;
}

bool Cyclogram::isModified() const
{
    return mModified;
}

void Cyclogram::setModified(bool isModified, bool sendSignal, bool recursive)
{
    mModified = isModified;

    if (recursive)
    {
        foreach (Command* command, mCommands)
        {
            if (command->type() == DRAKON::SUBPROGRAM)
            {
                CmdSubProgram* cmd = qobject_cast<CmdSubProgram*>(command);
                cmd->cyclogram()->setModified(isModified, sendSignal, recursive);
            }
        }
    }

    if (sendSignal)
    {
        emit modified();
    }
}

void Cyclogram::onCommandTextChanged(const QString& text)
{
    setModified(true, true, false);
}

void Cyclogram::variablesChanged()
{
    setModified(true, true, false);
}

void Cyclogram::variableCurrentValueChanged(const QString& name, qreal value)
{
}

void Cyclogram::variableInitialValueChanged(const QString& name, qreal value)
{
    variablesChanged();
}

void Cyclogram::setFirst(Command* first)
{
    mFirst = first;
}

void Cyclogram::setLast(Command* last)
{
    mLast = last;
}

bool Cyclogram::isMainCyclogram() const
{
    return mIsMainCyclogram;
}

void Cyclogram::setMainCyclogram(bool isMain)
{
    mIsMainCyclogram = isMain;
}

QString Cyclogram::defaultStorePath()
{
    return QDir::currentPath() + QString("/cyclograms/"); //TODO replace by application settings usage
}

void Cyclogram::changeCommandsOrder(Command* commandToMove, Command* commandAfter)
{
    if (mCommands.empty())
    {
        return;
    }

    int moveIndex = mCommands.indexOf(commandToMove);
    int afterIndex = mCommands.indexOf(commandAfter);

    if (moveIndex != -1)
    {
        mCommands.takeAt(moveIndex);
    }

    if (afterIndex != -1)
    {
        mCommands.insert(afterIndex + 1, commandToMove);
    }
}

QVariant Cyclogram::setting(const QString& name) const
{
    return mSettings.value(name, QVariant());
}

void Cyclogram::setSetting(const QString& key, const QVariant& value, bool sendSignal)
{
    if (key.isEmpty())
    {
        LOG_ERROR(QString("Trying to set setting with empty name"));
        return;
    }

    mSettings[key] = value;

    setModified(sendSignal, sendSignal, false);
}

const QMap<QString, QVariant>& Cyclogram::settings() const
{
    return mSettings;
}

void Cyclogram::onAppSettingsChanged()
{
    int commandExecutionDelay = AppSettings::instance().settingValue(AppSettings::COMMAND_EXECUTION_DELAY).toInt();

    foreach (Command* command, mCommands)
    {
        command->setExecutionDelay(commandExecutionDelay);
    }
}

Command* Cyclogram::createBranchCopy(Command* branch)
{
    Command* newBranch = Q_NULLPTR;

    newBranch = createCommand(branch->type());
    newBranch->copyFrom(branch);

    QString name = generateBranchName(branch->text());
    qobject_cast<CmdStateStart*>(newBranch)->setText(name);

    QMap<Command*, Command*> alreadyCreatedCommands;
    copyCommandTree(newBranch, branch, alreadyCreatedCommands);

    return newBranch;
}

void Cyclogram::copyCommandTree(Command* to, Command* from, QMap<Command*, Command*>& alreadyCreatedCommands)
{
    //alreadyCreatedCommands: key - existing command (template for copy), value - already created copy for "key" command

    QMap<Command*, Command*> existingMapping = alreadyCreatedCommands;

    //CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(from);
    if (from->type() == DRAKON::CYCLE) //IF-CYCLE
    {
        int TODO; // depending on role and shape type

        return;
    }
    else if (from->type() == DRAKON::CONDITION)
    {
        Command* down = from->nextCommand(ValencyPoint::Down);
        Command* right = from->nextCommand(ValencyPoint::Right);
        Command* underArrow = from->nextCommand(ValencyPoint::UnderArrow);

        if (underArrow)
        {
            Command* newUnderArrowCmd = Q_NULLPTR;

            auto it = existingMapping.find(underArrow);
            if (it == existingMapping.end()) // command was not created, create command copy
            {
                newUnderArrowCmd = copyCommand(to, underArrow, ValencyPoint::UnderArrow, existingMapping, true);
            }
            else
            {
                newUnderArrowCmd = it.value();
                // do not create further hierarchy, because it is already created
                to->replaceCommand(newUnderArrowCmd, from->role());
            }

            if (down)
            {
                if (down == underArrow) // straight line without down command
                {
                    to->replaceCommand(newUnderArrowCmd, ValencyPoint::Down); // just replace to the same command
                }
                else
                {
                    copyCommand(to, down, ValencyPoint::Down, existingMapping, false);
                }
            }

            if (right)
            {
                if (right == underArrow) // straight line without right command
                {
                    to->replaceCommand(newUnderArrowCmd, ValencyPoint::Right); // just replace to the same command
                }
                else
                {
                    copyCommand(to, right, ValencyPoint::Right, existingMapping, false);
                }
            }
        }

        return;
    }
    else if (from->type() == DRAKON::SELECT_STATE)
    {
        Command* down = from->nextCommand(ValencyPoint::Down);
        Command* right = from->nextCommand(ValencyPoint::Right);

        // "down" and "right" are always present, "underArrow" is never present
        if (down)
        {
            copyCommand(to, down, ValencyPoint::Down, existingMapping, false);
        }

        if (right)
        {
            copyCommand(to, right, ValencyPoint::Right, existingMapping, false);
        }

        return;
    }

    // simple commands
    auto nextCommands = from->nextCommands();
    for (auto it = nextCommands.begin(); it != nextCommands.end(); ++it)
    {
        Command* cmd = *it;
        if (!cmd)
        {
            continue;
        }

        Command* newNextCmd = Q_NULLPTR;

        auto iter = existingMapping.find(cmd);
        if (iter == existingMapping.end())// command was not created, create command copy
        {
            newNextCmd = copyCommand(to, cmd, ValencyPoint::Down, existingMapping, false);
        }
        else
        {
            newNextCmd = iter.value();
            to->replaceCommand(newNextCmd, ValencyPoint::Down);
        }
    }
}

Command* Cyclogram::copyCommand(Command* to, Command* from, int role, QMap<Command*, Command*>& alreadyCreatedCommands, bool addToMapping)
{
    Command* newCmd = createCommand(from->type());
    newCmd->copyFrom(from);
    to->replaceCommand(newCmd, ValencyPoint::Role(role));

    if (addToMapping)
    {
        alreadyCreatedCommands[from] = newCmd;
    }

    // if not GO_TO_BRANCH, move copying further
    if (newCmd->type() != DRAKON::GO_TO_BRANCH)
    {
        copyCommandTree(newCmd, from, alreadyCreatedCommands);
    }
    else // stop copy on branch end
    {
        newCmd->replaceCommand(from->nextCommand(), ValencyPoint::Down);
    }

    return newCmd;
}

QString Cyclogram::generateBranchName(const QString& templateName) const
{
    QList<Command*> branches;
    getBranches(branches);

    QString prefix = templateName;
    QString name = prefix;
    bool nameGenerated = false;
    int i = 1;
    while (!nameGenerated)
    {
        bool exist = false;
        foreach (Command* it, branches)
        {
            if (it->text() == name)
            {
                exist = true;
                break;
            }
        }

        nameGenerated = !exist;

        if (!nameGenerated)
        {
            name = prefix + QString(" ") + QString::number(i);
            ++i;
        }
    }

    return name;
}

bool Cyclogram::canBeCopied(DRAKON::IconType type)
{
    switch (type)
    {
    case DRAKON::TERMINATOR:
    case DRAKON::SELECT_STATE:
    case DRAKON::GO_TO_BRANCH:
        return false;

    default:
        break;
    }

    return true;
}

bool Cyclogram::isVariableUsed(const QString& name) const
{
    foreach (Command* command, mCommands)
    {
        if (command->isVariableUsed(name))
        {
            return true;
        }
    }

    return false;
}
