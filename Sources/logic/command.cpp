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

qint64 Command::smCounter = 0;

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

    mID = (QDateTime::currentMSecsSinceEpoch() + smCounter);
    ++smCounter;
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

    if (mType == DRAKON::GO_TO_BRANCH && role == ValencyPoint::Down)
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

    if (role < mNextCommands.size())
    {
        mNextCommands[role] = newCmd;
    }
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
        else if (questionCmd->questionType() == CmdQuestion::IF) // by default IF-type command refers with all branches to command below
        {
            questionCmd->replaceCommand(next, ValencyPoint::UnderArrow);
            questionCmd->replaceCommand(next, ValencyPoint::Down);
            questionCmd->replaceCommand(next, ValencyPoint::Right);
        }
        else if (questionCmd->questionType() == CmdQuestion::SWITCH_STATE)
        {
            questionCmd->replaceCommand(next, ValencyPoint::Down);
        }
    }
    else
    {
        newCmd->replaceCommand(next, role);
    }

    mNextCommands[role] = newCmd;
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
    writer->writeAttribute("id", QString::number(id()));

    writeCustomAttributes(writer);

    writer->writeEndElement(); // close command tag
}

void Command::read(QXmlStreamReader* reader)
{
    QXmlStreamAttributes attributes = reader->attributes();

    if (attributes.hasAttribute("id"))
    {
        mID = attributes.value("id").toString().toLongLong();
    }

    readCustomAttributes(reader);
}

void Command::writeCustomAttributes(QXmlStreamWriter* writer)
{
}

void Command::readCustomAttributes(QXmlStreamReader* reader)
{
}

qint64 Command::id() const
{
    return mID;
}
