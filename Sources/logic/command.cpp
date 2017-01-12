#include <QTime>
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

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
    mRole(ValencyPoint::Down),
    mFlags(Command::All),
    mHasError(false),
    mExecutionDelay(0),
    mVarCtrl(Q_NULLPTR),
    mSystemState(Q_NULLPTR)
{
    setExecutionDelay(EXECUTION_DELAY);

    for (int i = 0; i < childCmdCnt; ++i)
    {
        mNextCommands.push_back(Q_NULLPTR);
    }

    mID = QDateTime::currentMSecsSinceEpoch();
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
    emit finished(nextCommand());
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

void Command::replaceCommand(Command *newCmd, ValencyPoint::Role role)
{
    if (newCmd)
    {
        newCmd->setRole(role);
    }

    if (mType == DRAKON::GO_TO_BRANCH)
    {
        if (newCmd)
        {
            if (nextCommand(role))
            {
                disconnect(nextCommand(role), SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            }

            connect(newCmd, SIGNAL(textChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            onNextCmdTextChanged(newCmd->text());

            if (hasError()) // error fixing after branch deletion
            {
                setErrorStatus(false);
            }
        }
        else // branch deletion
        {
            setErrorStatus(true);
        }
    }

    mNextCommands[role] = newCmd;
}

void Command::replaceCommand(Command* newCmd, Command* oldCmd)
{
    int TODO; // used in QUESTION command deletion

    for (int i = 0, sz = mNextCommands.size(); i < sz; ++i)
    {
        if (mNextCommands[i] && mNextCommands[i] == oldCmd)
        {
            if (i == ValencyPoint::UnderArrow)
            {
                Command* right = mNextCommands[ValencyPoint::Right];
                if (right)
                {
                    replaceReferences(oldCmd, newCmd, right);
                }

                Command* down = mNextCommands[ValencyPoint::Down];
                if (down)
                {
                    replaceReferences(oldCmd, newCmd, down);
                }
            }

            newCmd->setRole(mNextCommands[i]->role());
            mNextCommands[i] = newCmd;
            return;
        }
    }
}

void Command::onNextCmdTextChanged(const QString& text)
{
    mText = text;
    emit textChanged(mText);
}

ValencyPoint::Role Command::role() const
{
    return mRole;
}

void Command::setRole(ValencyPoint::Role role)
{
    mRole = role;
}

void Command::setActive(bool active)
{
    emit activeStateChanged(active);
}

bool Command::hasError() const
{
    return mHasError;
}

void Command::insertCommand(Command* newCmd, ValencyPoint::Role role)
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

    if (!newCmd)
    {
        return;
    }

    newCmd->setRole(role);

    if (mType == DRAKON::QUESTION) // insertion in QUSTION command
    {
        CmdQuestion* thisCmd = qobject_cast<CmdQuestion*>(this);
        Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
        Command* right = nextCommand(ValencyPoint::Right);
        Command* down = nextCommand(ValencyPoint::Down);

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
                        Command* cmd = (underArrow == this) ? this : underArrow;

                        newCmd->replaceCommand(cmd, ValencyPoint::Down);
                        newCmd->replaceCommand(newCmd, ValencyPoint::Right);
                        newCmd->replaceCommand(newCmd, ValencyPoint::UnderArrow);

                        mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                        // update "right" branch
                        if (right == this)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(cmd, newCmd, right);
                        }
                    }
                    else
                    {
                        newCmd->replaceCommand(mNextCommands[role], ValencyPoint::Down);
                        newCmd->replaceCommand(newCmd, ValencyPoint::Right);
                        newCmd->replaceCommand(newCmd, ValencyPoint::UnderArrow);

                        mNextCommands[role] = newCmd;
                    }
                }
                else // QUESTION-IF insertion in QUESTION-CYCLE
                {
                    if (role == ValencyPoint::UnderArrow)
                    {
                        // update "under arrow" branch
                        Command* cmd = (underArrow == this) ? this : underArrow;

                        newCmd->replaceCommand(cmd, ValencyPoint::Down);
                        newCmd->replaceCommand(cmd, ValencyPoint::Right);
                        newCmd->replaceCommand(cmd, ValencyPoint::UnderArrow);

                        mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                        // update "right" branch
                        if (right == this)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(cmd, newCmd, right);
                        }
                    }
                    else
                    {
                        newCmd->replaceCommand(mNextCommands[role], ValencyPoint::Down);
                        newCmd->replaceCommand(mNextCommands[role], ValencyPoint::Right);
                        newCmd->replaceCommand(mNextCommands[role], ValencyPoint::UnderArrow);

                        mNextCommands[role] = newCmd;
                    }
                }
            }
            else // simple command insertion in QUESTION-CYCLE
            {
                if (role == ValencyPoint::UnderArrow)
                {
                    // update "under arrow" branch
                    Command* cmd = (underArrow == this) ? this : underArrow;

                    newCmd->replaceCommand(cmd, ValencyPoint::Down);
                    mNextCommands[ValencyPoint::UnderArrow] = newCmd;

                    // update "right" branch
                    if (right == this)
                    {
                        mNextCommands[ValencyPoint::Right] = newCmd;
                    }
                    else
                    {
                        replaceReferences(cmd, newCmd, right);
                    }
                }
                else
                {
                    newCmd->replaceCommand(mNextCommands[role], ValencyPoint::Down);
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
                        if (down == underArrow)
                        {
                            mNextCommands[ValencyPoint::Down] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, down);
                        }

                        if (right == underArrow)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, right);
                        }

                        // old "under arrow" command now become "down" command for new command, "right" and "under arrow" link command to itself
                        newCmd->replaceCommand(underArrow, ValencyPoint::Down);
                    }
                    else
                    {
                        Command* existing = mNextCommands[role];
                        Command* cmd = (existing == underArrow) ? underArrow : existing;

                        newCmd->replaceCommand(cmd, ValencyPoint::Down);
                    }

                    newCmd->replaceCommand(newCmd, ValencyPoint::Right);
                    newCmd->replaceCommand(newCmd, ValencyPoint::UnderArrow);
                    mNextCommands[role] = newCmd;
                }
                else // QUESTION-IF insertion in QUESTION-IF
                {
                    Command* cmd = Q_NULLPTR;

                    if (role == ValencyPoint::UnderArrow)
                    {
                        cmd = underArrow;

                        // recursively replace references to current "under arrow" command with new "under arrow" command
                        if (down == underArrow)
                        {
                            mNextCommands[ValencyPoint::Down] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, down);
                        }

                        if (right == underArrow)
                        {
                            mNextCommands[ValencyPoint::Right] = newCmd;
                        }
                        else
                        {
                            replaceReferences(underArrow, newCmd, right);
                        }
                    }
                    else
                    {
                        Command* existing = mNextCommands[role];
                        cmd = (existing == underArrow) ? underArrow : existing;
                    }

                    // old "under arrow" command now become "under arrow, etc" command for new command
                    newCmd->replaceCommand(cmd, ValencyPoint::Down);
                    newCmd->replaceCommand(cmd, ValencyPoint::Right);
                    newCmd->replaceCommand(cmd, ValencyPoint::UnderArrow);
                    mNextCommands[role] = newCmd;
                }
            }
            else // simple command insertion in QUESTION-IF
            {
                if (role == ValencyPoint::UnderArrow)
                {
                    // recursively replace references to current "under arrow" command with new "under arrow" command
                    if (down == underArrow)
                    {
                        mNextCommands[ValencyPoint::Down] = newCmd;
                    }
                    else
                    {
                        replaceReferences(underArrow, newCmd, down);
                    }

                    if (right == underArrow)
                    {
                        mNextCommands[ValencyPoint::Right] = newCmd;
                    }
                    else
                    {
                        replaceReferences(underArrow, newCmd, right);
                    }

                    newCmd->replaceCommand(underArrow, ValencyPoint::Down);
                }
                else
                {
                    Command* existing = mNextCommands[role];
                    Command* cmd = (existing == underArrow) ? underArrow : existing;

                    newCmd->replaceCommand(cmd, ValencyPoint::Down);
                }

                mNextCommands[role] = newCmd;
            }
        }
    }
    else // QUESTION insertion in simple command
    {
        Command* next = nextCommand();

        if (newCmd->type() == DRAKON::QUESTION)
        {
            CmdQuestion* questionCmd = qobject_cast<CmdQuestion*>(newCmd);
            if (questionCmd->questionType() == CmdQuestion::CYCLE)
            {
                questionCmd->replaceCommand(next, ValencyPoint::Down);
                questionCmd->replaceCommand(questionCmd, ValencyPoint::UnderArrow);
                questionCmd->replaceCommand(questionCmd, ValencyPoint::Right);
            }
            else // by default IF-type command refers with all branches to command below
            {
                questionCmd->replaceCommand(next, ValencyPoint::UnderArrow);
                questionCmd->replaceCommand(next, ValencyPoint::Down);
                questionCmd->replaceCommand(next, ValencyPoint::Right);
            }
        }
        else
        {
            newCmd->replaceCommand(next, role);
        }

        mNextCommands[role] = newCmd;
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

void Command::setSystemState(SystemState* state)
{
    mSystemState = state;
}

SystemState* Command::systemState() const
{
    return mSystemState;
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

Command* Command::nextCommand(ValencyPoint::Role role) const
{
    if (role >= ValencyPoint::Down && role < mNextCommands.size())
    {
        return mNextCommands[role];
    }

    return Q_NULLPTR;
}

void Command::write(QXmlStreamWriter* writer)
{
    static QMetaEnum metaEnum;
    if (metaEnum.keyCount() == 0)
    {
        metaEnum = QMetaEnum::fromType<DRAKON::IconType>();
    }

    writer->writeStartElement("command"); // open command tag

    // write common commands attributes
    writer->writeAttribute("type", metaEnum.valueToKey(type()));

    writeCustomAttributes(writer);

    // write command links
    writer->writeStartElement("next_commands");

    Command* underArrow = nextCommand(ValencyPoint::UnderArrow);
    Command* down = nextCommand(ValencyPoint::Down);
    Command* right = nextCommand(ValencyPoint::Right);

    if (underArrow)
    {
        writer->writeAttribute("under_arrow", QString::number(underArrow->id()));
    }

    if (down)
    {
        writer->writeAttribute("down", QString::number(down->id()));
    }

    if (right)
    {
        writer->writeAttribute("right", QString::number(right->id()));
    }

    writer->writeEndElement();

    writer->writeEndElement(); // close command tag
}

void Command::read(QXmlStreamReader* reader)
{
    int TODO;
}

void Command::writeCustomAttributes(QXmlStreamWriter* writer)
{
    int TODO;// reimplement in inherited commmands classes
}

void Command::readCustomAttributes(QXmlStreamReader* reader)
{
    int TODO;// reimplement in inherited commmands classes
}

qint64 Command::id() const
{
    return mID;
}
