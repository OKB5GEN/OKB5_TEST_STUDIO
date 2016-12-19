#include <QTime>
#include <QTimer>

#include "Headers/logic/command.h"
#include "Headers/logic/variable_controller.h"

#include "Headers/logic/commands/cmd_question.h" // TODO remove
#include "Headers/gui/cyclogram/valency_point.h"

namespace
{
    static const int EXECUTION_DELAY = 50;
}

Command::Command(DRAKON::IconType type, int childCmdCnt, QObject * parent):
    QObject(parent),
    mType(type),
    mRole(0),
    mFlags(Command::All),
    mParentCommand(Q_NULLPTR),
    mHasError(false),
    mExecutionDelay(0)
{
    setExecutionDelay(EXECUTION_DELAY);

    for (int i = 0; i < childCmdCnt; ++i)
    {
        mChildCommands.push_back(Q_NULLPTR);
    }
}

Command::~Command()
{

}

void Command::run()
{
    if (mExecutionDelay > 0)
    {
        QTimer::singleShot(mExecutionDelay, this, SLOT(end()));
    }
    else
    {
        end();
    }
}

void Command::end()
{
    if (mNextCommands.empty())
    {
        emit finished(Q_NULLPTR);
        return;
    }

    emit finished(mNextCommands[0]);
}

void Command::stop()
{

}

void Command::pause()
{

}

void Command::resume()
{

}

DRAKON::IconType Command::type() const
{
    return mType;
}

const QString& Command::text() const
{
    return mText;
}

const QString& Command::errorDesc() const
{
    return mErrorText;
}

void Command::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t Command::flags() const
{
    return mFlags;
}

const QList<Command*>& Command::nextCommands() const
{
    return mNextCommands;
}

const QList<Command*>& Command::childCommands() const
{
    return mChildCommands;
}

void Command::addCommand(Command* cmd, int role /*= 0*/)
{
    if (!cmd)
    {
        return;
    }

    if (cmd->type() != DRAKON::BRANCH_BEGIN)
    {
        cmd->setParentCommand(this);
    }

    cmd->setRole(role);
    mNextCommands.push_back(cmd);

    if (mType == DRAKON::GO_TO_BRANCH && cmd->type() == DRAKON::BRANCH_BEGIN)
    {
        connect(cmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
        onNextCmdTextChanged(cmd->text());
    }

    if (mType != DRAKON::GO_TO_BRANCH && mType != DRAKON::TERMINATOR)
    {
        setChildCommand(cmd, role);
    }
}

void Command::onNextCmdTextChanged(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}

int Command::role() const
{
    return mRole;
}

void Command::setRole(int role)
{
    mRole = role;
}

void Command::setActive(bool active)
{
    emit activeStateChanged(active);
}

void Command::setParentCommand(Command* cmd)
{
    mParentCommand = cmd;
}

Command* Command::parentCommand() const
{
    int TODO; // неясно которую из команд считать парентовой в случае если сверху находится развилка QUESTION или SWICH-CASE, вероятно будет тоже массив, как и "nextCommands"
    return mParentCommand;
}

bool Command::hasError() const
{
    return mHasError;
}

void Command::insertCommand(Command* newCmd, int role)
{
    // TODO very complex function, split it on small

    // LOGIC of this function
    // insertion in QUESTION
    //     insertion in QUESTION-CYCLE
    //         insertion of QUESTION-CYCLE
    //         insertion of QUESTION-IF
    //         insertion of "one-cell" command
    //     insertion in QUESTION-IF
    //         insertion of QUESTION-CYCLE
    //         insertion of QUESTION-IF
    //         insertion of "one-cell" command
    // insertion in "one-cell" command
    //     insertion of QUESTION
    //     insertion of "one-cell" command

    if (mType == DRAKON::QUESTION) // insertion in QUSTION command
    {
        newCmd->setParentCommand(this);
        newCmd->setRole(role);
        setChildCommand(newCmd, role);

        CmdQuestion* thisCmd = qobject_cast<CmdQuestion*>(this);
        Command* underArrow = mNextCommands[ValencyPoint::UnderArrow];

        if (thisCmd->questionType() == CmdQuestion::CYCLE) // insertions in QUESTION-CYCLE
        {
            if (newCmd->type() == DRAKON::QUESTION)
            {
                CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(newCmd);

                if (questionCmd->questionType() == CmdQuestion::CYCLE) // QUESTION-CYCLE insertion in QUESTION-CYCLE
                {
                    if (role == ValencyPoint::UnderArrow)
                    {
                        // update "under arrow" branch
                        Command* right = mNextCommands[ValencyPoint::Right];
                        Command* cmd = (underArrow == this) ? this : underArrow;

                        newCmd->addCommand(cmd, ValencyPoint::Down);
                        newCmd->addCommand(newCmd, ValencyPoint::Right);
                        newCmd->addCommand(newCmd, ValencyPoint::UnderArrow);

                        mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                        // update "right" branch
                        if (right == this)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            Command* tree = mNextCommands[ValencyPoint::Right];
                            replaceReferences(cmd, newCmd, tree);
                        }
                    }
                    else
                    {
                        newCmd->addCommand(mNextCommands[role], ValencyPoint::Down);
                        newCmd->addCommand(newCmd, ValencyPoint::Right);
                        newCmd->addCommand(newCmd, ValencyPoint::UnderArrow);

                        mNextCommands[role] = newCmd;
                    }
                }
                else // QUESTION-IF insertion in QUESTION-CYCLE
                {
                    if (role == ValencyPoint::UnderArrow)
                    {
                        // update "under arrow" branch
                        Command* right = mNextCommands[ValencyPoint::Right];
                        Command* cmd = (underArrow == this) ? this : underArrow;

                        newCmd->addCommand(cmd, ValencyPoint::Down);
                        newCmd->addCommand(cmd, ValencyPoint::Right);
                        newCmd->addCommand(cmd, ValencyPoint::UnderArrow);

                        mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                        // update "right" branch
                        if (right == this)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            Command* tree = mNextCommands[ValencyPoint::Right];
                            replaceReferences(cmd, newCmd, tree);
                        }
                    }
                    else
                    {
                        newCmd->addCommand(mNextCommands[role], ValencyPoint::Down);
                        newCmd->addCommand(mNextCommands[role], ValencyPoint::Right);
                        newCmd->addCommand(mNextCommands[role], ValencyPoint::UnderArrow);

                        mNextCommands[role] = newCmd;
                    }
                }
            }
            else // simple command insertion in QUESTION-CYCLE
            {
                if (role == ValencyPoint::UnderArrow)
                {
                    // update "under arrow" branch
                    Command* right = mNextCommands[ValencyPoint::Right];
                    Command* cmd = (underArrow == this) ? this : underArrow;

                    newCmd->addCommand(cmd, ValencyPoint::Down);
                    mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                    // update "right" branch
                    if (right == this)
                    {
                        mNextCommands[ValencyPoint::Right] = newCmd;
                    }
                    else
                    {
                        Command* tree = mNextCommands[ValencyPoint::Right];
                        replaceReferences(cmd, newCmd, tree);
                    }
                }
                else
                {
                    newCmd->addCommand(mNextCommands[role], ValencyPoint::Down);
                    mNextCommands[role] = newCmd;
                }
            }
        }
        else // insertions in QUESTION-IF
        {
            if (newCmd->type() == DRAKON::QUESTION)
            {
                CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(newCmd);

                if (questionCmd->questionType() == CmdQuestion::CYCLE) // QUESTION-CYCLE insertion in QUESTION-IF
                {
                    if (role == ValencyPoint::UnderArrow)
                    {
                        // recursively replace references to current "under arrow" command with new "under arrow" command
                        Command* tree1 = mNextCommands[ValencyPoint::Down];
                        if (tree1 == underArrow)
                        {
                            mNextCommands[ValencyPoint::Down] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, tree1);
                        }

                        Command* tree2 = mNextCommands[ValencyPoint::Right];
                        if (tree2 == underArrow)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, tree2);
                        }

                        // old "under arrow" command now become "down" command for new command, "right" and "under arrow" link command to itself
                        newCmd->addCommand(underArrow, ValencyPoint::Down);
                    }
                    else
                    {
                        Command* existing = mNextCommands[role];
                        Command* cmd = (existing == underArrow) ? underArrow : existing;

                        newCmd->addCommand(cmd, ValencyPoint::Down);
                    }

                    newCmd->addCommand(newCmd, ValencyPoint::Right);
                    newCmd->addCommand(newCmd, ValencyPoint::UnderArrow);
                    mNextCommands[role] = newCmd;
                }
                else // QUESTION-IF insertion in QUESTION-IF
                {
                    Command* cmd = Q_NULLPTR;

                    if (role == ValencyPoint::UnderArrow)
                    {
                        cmd = underArrow;

                        // recursively replace references to current "under arrow" command with new "under arrow" command
                        Command* tree1 = mNextCommands[ValencyPoint::Down];
                        if (tree1 == underArrow)
                        {
                            mNextCommands[ValencyPoint::Down] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, tree1);
                        }

                        Command* tree2 = mNextCommands[ValencyPoint::Right];
                        if (tree2 == underArrow)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, tree2);
                        }
                    }
                    else
                    {
                        Command* existing = mNextCommands[role];
                        cmd = (existing == underArrow) ? underArrow : existing;
                    }

                    // old "under arrow" command now become "under arrow, etc" command for new command
                    newCmd->addCommand(cmd, ValencyPoint::Down);
                    newCmd->addCommand(cmd, ValencyPoint::Right);
                    newCmd->addCommand(cmd, ValencyPoint::UnderArrow);
                    mNextCommands[role] = newCmd;
                }
            }
            else // simple command insertion in QUESTION-IF
            {
                if (role == ValencyPoint::UnderArrow)
                {
                    // recursively replace references to current "under arrow" command with new "under arrow" command
                    Command* tree1 = mNextCommands[ValencyPoint::Down];
                    if (tree1 == underArrow)
                    {
                        mNextCommands[ValencyPoint::Down] = newCmd;
                    }
                    else
                    {
                        replaceReferences(underArrow, newCmd, tree1);
                    }

                    Command* tree2 = mNextCommands[ValencyPoint::Right];
                    if (tree2 == underArrow)
                    {
                        mNextCommands[ValencyPoint::Right] = newCmd;
                    }
                    else
                    {
                        replaceReferences(underArrow, newCmd, tree2);
                    }

                    newCmd->addCommand(underArrow, ValencyPoint::Down);
                }
                else
                {
                    Command* existing = mNextCommands[role];
                    Command* cmd = (existing == underArrow) ? underArrow : existing;

                    newCmd->addCommand(cmd, ValencyPoint::Down);
                }

                mNextCommands[role] = newCmd;
            }
        }
    }
    else // QUESTION insertion in simple command
    {
        for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
        {
            if (mNextCommands[i]->role() == role)
            {
                setChildCommand(newCmd, role);
                newCmd->setParentCommand(this);
                newCmd->setRole(role);
                if (newCmd->type() == DRAKON::QUESTION)
                {
                    CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(newCmd);
                    if (questionCmd->questionType() == CmdQuestion::CYCLE)
                    {
                        questionCmd->addCommand(mNextCommands[i], ValencyPoint::Down);
                        questionCmd->addCommand(questionCmd, ValencyPoint::UnderArrow);
                        questionCmd->addCommand(questionCmd, ValencyPoint::Right);
                    }
                    else // by default IF-type command refers with all branches to command below
                    {
                        questionCmd->addCommand(mNextCommands[i], ValencyPoint::UnderArrow);
                        questionCmd->addCommand(mNextCommands[i], ValencyPoint::Down);
                        questionCmd->addCommand(mNextCommands[i], ValencyPoint::Right);
                    }
                }
                else
                {
                    newCmd->addCommand(mNextCommands[i], role);
                }

                mNextCommands[i] = newCmd;
                break;
            }
        }
    }
}

void Command::setChildCommand(Command* cmd, int role)
{
    if (role >= 0 && role < mChildCommands.size())
    {
        mChildCommands[role] = cmd;
    }
}

void Command::replaceChildCommand(Command* newCmd, Command* oldCmd)
{
    for (int i = 0, sz = mChildCommands.size(); i < sz; ++i)
    {
        if (mChildCommands[i] == oldCmd)
        {
            mChildCommands[i] = newCmd;
            return;
        }
    }
}

void Command::replaceCommand(Command *newCmd, int role)
{
    int TODO2; // possibly make virtual and move to DRAKON::GO_TO_BRANCH class

    if (mType == DRAKON::GO_TO_BRANCH)
    {
        if (!newCmd)  // branch deletion
        {
            mNextCommands.clear();
            setErrorStatus(true);
            return;
        }

        if (mNextCommands.empty() && hasError()) // error fixing after branch deletion
        {
            newCmd->setRole(role); //TODO possibly not?
            mNextCommands.push_back(newCmd);
            connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            setErrorStatus(false);
            return;
        }
    }

    int TODO; // непонятно как эти роли должны передаваться при замене
    // роль - это актуально только для ветвлений
    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i]->role() == role)
        {
            if (mType == DRAKON::GO_TO_BRANCH && newCmd->type() == DRAKON::BRANCH_BEGIN)
            {
                disconnect(mNextCommands[i], SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
                connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            }
            else
            {
                newCmd->setParentCommand(this);
            }

            mNextCommands[i] = newCmd;
            break;
        }
    }
}

void Command::setErrorStatus(bool status)
{
    mHasError = status;
    emit errorStatusChanged(mHasError);
}

void Command::setExecutionDelay(int msec)
{
    mExecutionDelay = msec;
}

void Command::setVariableController(VariableController* controller)
{
    mVarCtrl = controller;

    Qt::ConnectionType connection = Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection);
    connect(mVarCtrl, SIGNAL(nameChanged(const QString&,const QString&)), this, SLOT(onNameChanged(const QString&, const QString&)), connection);
    connect(mVarCtrl, SIGNAL(variableRemoved(const QString&)), this, SLOT(onVariableRemoved(const QString&)), connection);
}

VariableController* Command::variableController() const
{
    return mVarCtrl;
}

void Command::onNameChanged(const QString& newName, const QString& oldName)
{

}

void Command::onVariableRemoved(const QString& name)
{

}

void Command::replaceReferences(Command* oldCmd, Command* newCmd, Command* tree)
{
    if (!tree || !oldCmd)
    {
        qDebug("Command::replaceReferences: incorrect input 1");
        return;
    }

    if (tree == oldCmd)
    {
        qDebug("Command::replaceReferences: incorrect input 2");
        return;
    }

    if (tree->type() == DRAKON::GO_TO_BRANCH || tree->type() == DRAKON::TERMINATOR)
    {
        return;
    }

    const QList<Command*>& commands = tree->nextCommands();
    for (int i = 0, sz = commands.size(); i < sz; ++i)
    {
        if (commands[i] == oldCmd)
        {
            tree->replaceCommand(newCmd, commands[i]->role());
        }
        else
        {
            replaceReferences(oldCmd, newCmd, commands[i]);
        }
    }
}
