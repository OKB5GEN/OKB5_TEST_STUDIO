#include <QTime>
#include <QTimer>
#include <QMetaEnum>
#include <QDir>

#include "Headers/logic/cyclogram.h"
#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logic/commands/cmd_set_state.h"
#include "Headers/logic/commands/cmd_title.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/logic/commands/cmd_action.h"
#include "Headers/logic/commands/cmd_action_math.h"
#include "Headers/logic/commands/cmd_action_module.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/logic/commands/cmd_sub_program.h"

#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include "Headers/system/system_state.h"
#include "Headers/file_reader.h"

namespace
{
    static const int COMMAND_RUN_INTERVAL = 10; // msec

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
    mState(STOPPED),
    mVarController(Q_NULLPTR),
    mSystemState(Q_NULLPTR),
    mIsMainCyclogram(false),
    mModified(false)
{
    mVarController = new VariableController(this);

    connect(mVarController, SIGNAL(variableAdded(const QString&, qreal)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(variableRemoved(const QString&)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(initialValueChanged(const QString&, qreal)), this, SLOT(variableInitialValueChanged(const QString&, qreal)));
    connect(mVarController, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(variableCurrentValueChanged(const QString&, qreal)));
    connect(mVarController, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(variablesChanged()));
    connect(mVarController, SIGNAL(descriptionChanged(const QString&, const QString&)), this, SLOT(variablesChanged()));
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

    emit changed();
}

void Cyclogram::run()
{
    LOG_INFO("==================================");
    if (mIsMainCyclogram)
    {
        LOG_INFO("Cyclogram started");
    }
    else
    {
        LOG_INFO("Subprogram started");
    }

    LOG_INFO("==================================");

    if (mState == STOPPED && mFirst != Q_NULLPTR)
    {
        mCurrent = mFirst;

        if (mIsMainCyclogram)
        {
            mVarController->restart(); // set current values to initial values if it is main cyclogram
            mSystemState->onCyclogramStart();

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
}

void Cyclogram::onCommandFinished(Command* cmd)
{
    mCurrent->setActive(false);
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    LogCmd(mCurrent, "finished");

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
        LOG_INFO("==================================");
        if (mIsMainCyclogram)
        {
            LOG_INFO("Main cyclogram finished");
        }
        else
        {
            LOG_INFO("Subprogram finished");
        }
        LOG_INFO("==================================");

        stop();
        emit finished("");
    }
}

void Cyclogram::onCriticalError(Command* cmd)
{
    disconnect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
    disconnect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));
    LogCmd(mCurrent, "critical error: " + cmd->errorDesc());

    LOG_ERROR("==================================");
    if (mIsMainCyclogram)
    {
        LOG_ERROR("Cyclogram stopped due to critical runtime error");
    }
    else
    {
        LOG_ERROR("Subprogram stopped due to critical runtime error");
    }

    LOG_ERROR("==================================");

    for (auto it = mVarController->variablesData().begin(); it != mVarController->variablesData().end(); ++it)
    {
        LOG_ERROR(QString("Variable '%1' current value is %2").arg(it.key()).arg(it.value().currentValue));
    }

    stop();
    emit finished(tr("Critical error occured: ") + cmd->errorDesc());
}

void Cyclogram::runCurrentCommand()
{
    if (mCurrent)
    {
        connect(mCurrent, SIGNAL(finished(Command*)), this, SLOT(onCommandFinished(Command*)));
        connect(mCurrent, SIGNAL(criticalError(Command*)), this, SLOT(onCriticalError(Command*)));

        LogCmd(mCurrent, "started");
        mCurrent->setActive(true);
        mCurrent->run();
    }
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
    }

    mCurrent = mFirst;
    setState(STOPPED);

    if (mIsMainCyclogram && mSystemState)
    {
        mSystemState->setDefaultState();
    }
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

void Cyclogram::deleteCommandTree(Command* cmd, bool silent)
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
        if (cmd->nextCommands()[i])
        {
            deleteCommandTree(cmd->nextCommands()[i], silent);
        }
    }

    deleteCommandImpl(cmd, silent);
}

void Cyclogram::deleteCommand(Command* cmd, bool recursive /*= false*/)
{
    if (recursive)
    {
        deleteCommandTree(cmd, false);
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
    disconnect(cmd, SIGNAL(textChanged(const QString&)), this, SLOT(onCommandTextChanged(QString)));
    cmd->deleteLater();
    setModified(true, !silent); // modified on command deletion
}

Command* Cyclogram::createCommand(DRAKON::IconType type, int param /*= -1*/)
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
            cmd = new CmdSubProgram(this);
        }
        break;
    case DRAKON::ACTION_MATH:
        {
            cmd = new CmdActionMath(this);
        }
        break;

    case DRAKON::QUESTION:
        {
            CmdQuestion* tmp = new CmdQuestion(this);
            tmp->setQuestionType(CmdQuestion::QuestionType(param));
            cmd = tmp;
        }
        break;
    case DRAKON::ACTION_MODULE:
        {
            cmd = new CmdActionModule(this);
        }
        break;

    default:
        LOG_ERROR(QString("Command %1 not implemented").arg(int(type)));
        break;
    }

    if (cmd)
    {
        cmd->setVariableController(mVarController);
        cmd->setSystemState(mSystemState);

        mCommands.push_back(cmd);
        connect(cmd, SIGNAL(textChanged(const QString&)), this, SLOT(onCommandTextChanged(QString)));
        setModified(true, true); // modified on command adding
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

    LOG_DEBUG("Cyclogram state changed to %s", qUtf8Printable(text));

    mState = state;
    emit stateChanged(mState);
}

Command* Cyclogram::validate() const
{
    foreach (Command* command, mCommands)
    {
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

void Cyclogram::setModified(bool isModified, bool sendSignal)
{
    mModified = isModified;

    if (sendSignal)
    {
        emit modified();
    }
}

void Cyclogram::onCommandTextChanged(const QString& text)
{
    setModified(true, true);
}

void Cyclogram::variablesChanged()
{
    setModified(true, true);
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
    return QDir::currentPath() + QString("/cyclograms/");
}

//bool Cyclogram::load(const QString& fileName)
//{
//    clear();

//    QFile file(fileName);
//    FileReader reader(this);

//    if (!file.open(QFile::ReadOnly | QFile::Text))
//    {
//        LOG_ERROR(QString("Cannot open file %1: %2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
//        createDefault();
//        return false;
//    }

//    if (!reader.read(&file))
//    {
//        LOG_ERROR(QString("Parse error in file %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
//        createDefault();
//        return false;
//    }

//    return true;
//}
