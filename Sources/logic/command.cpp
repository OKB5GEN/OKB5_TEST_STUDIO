#include <QTime>
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

#include "Headers/logic/command.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/commands/cmd_question.h" // TODO remove
#include "Headers/gui/cyclogram/valency_point.h"
#include "Headers/app_settings.h"

namespace
{
}

qint64 Command::smCounter = 0;

Command::Command(DRAKON::IconType type, int childCmdCnt, QObject * parent):
    QObject(parent),
    mType(type),
    mRole(ValencyPoint::Down),
    mFlags(Command::All),
    mOnStartTextColor(0xff000000), // black by default
    mOnFinishTextColor(0xff000000), // black by default
    mHasError(false),
    mExecutionDelay(0),
    mVarCtrl(Q_NULLPTR),
    mSystemState(Q_NULLPTR)
{
    setExecutionDelay(AppSettings::instance().settingValue(AppSettings::COMMAND_EXECUTION_DELAY).toInt());

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

#ifdef ENABLE_CYCLOGRAM_PAUSE
void Command::pause()
{

}

void Command::resume()
{

}
#endif

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
                disconnect(nextCommand(role), SIGNAL(dataChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
            }

            connect(newCmd, SIGNAL(dataChanged(const QString&)), this, SLOT(onNextCmdTextChanged(const QString&)));
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
    emit dataChanged(mText);
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
    updateText();
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
        LOG_WARNING(QString("Command::replaceReferences: incorrect input 1"));
        return;
    }

    if (tree == oldCmd)
    {
        LOG_WARNING(QString("Command::replaceReferences: incorrect input 2"));
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
    writer->writeAttribute("on_start_text", mOnStartConsoleText);
    writer->writeAttribute("on_finish_text", mOnFinishConsoleText);
    writer->writeAttribute("on_start_text_color", QString::number(mOnStartTextColor, 16));
    writer->writeAttribute("on_finish_text_color", QString::number(mOnFinishTextColor, 16));

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

    if (attributes.hasAttribute("on_start_text"))
    {
        mOnStartConsoleText = attributes.value("on_start_text").toString();
    }

    if (attributes.hasAttribute("on_finish_text"))
    {
        mOnFinishConsoleText = attributes.value("on_finish_text").toString();
    }

    bool ok;
    if (attributes.hasAttribute("on_start_text_color"))
    {
        mOnStartTextColor = attributes.value("on_start_text_color").toULong(&ok, 16);
    }

    if (attributes.hasAttribute("on_finish_text_color"))
    {
        mOnFinishTextColor = attributes.value("on_finish_text_color").toULong(&ok, 16);
    }

    readCustomAttributes(reader);
}

void Command::writeCustomAttributes(QXmlStreamWriter* writer)
{
}

void Command::readCustomAttributes(QXmlStreamReader* reader)
{
}

void Command::updateText()
{
}

qint64 Command::id() const
{
    return mID;
}

const QString& Command::onStartConsoleText() const
{
    return mOnStartConsoleText;
}

const QString& Command::onFinishConsoleText() const
{
    return mOnFinishConsoleText;
}

void Command::setConsoleMessageData(const QString& beforeText, const QString& afterText, uint32_t beforeTextColorARGB, uint32_t afterTextColorARGB)
{
    QString startTextBefore = mOnStartConsoleText;
    QString endTextBefore = mOnFinishConsoleText;
    uint32_t startColorBefore = mOnStartTextColor;
    uint32_t endColorBefore = mOnFinishTextColor;

    mOnFinishConsoleText = afterText;
    mOnStartConsoleText = beforeText;
    mOnStartTextColor = beforeTextColorARGB;
    mOnFinishTextColor = afterTextColorARGB;

    bool isDataChanged = ((startTextBefore != mOnStartConsoleText)
                          || (endTextBefore != mOnFinishConsoleText)
                          || (startColorBefore != mOnStartTextColor)
                          || (endColorBefore != mOnFinishTextColor));

    if (isDataChanged)
    {
        emit dataChanged(mText);
    }
}

void Command::setOnStartConsoleText(const QString& text)
{
    QString textBefore = mOnStartConsoleText;
    mOnStartConsoleText = text;
    if (textBefore != mOnStartConsoleText)
    {
        emit dataChanged(mText);
    }
}

void Command::setOnFinishConsoleText(const QString& text)
{
    QString textBefore = mOnFinishConsoleText;
    mOnFinishConsoleText = text;
    if (textBefore != mOnFinishConsoleText)
    {
        emit dataChanged(mText);
    }
}

void Command::setOnStartConsoleTextColor(uint32_t argb)
{
    uint32_t colorBefore = mOnStartTextColor;
    mOnStartTextColor = argb;
    if (colorBefore != mOnStartTextColor)
    {
        emit dataChanged(mText);
    }
}

void Command::setOnFinishConsoleTextColor(uint32_t argb)
{
    uint32_t colorBefore = mOnFinishTextColor;
    mOnFinishTextColor = argb;
    if (colorBefore != mOnFinishTextColor)
    {
        emit dataChanged(mText);
    }
}

QColor Command::onStartConsoleTextColor() const
{
    return QColor::fromRgba(mOnStartTextColor);
}

QColor Command::onFinishConsoleTextColor() const
{
    return QColor::fromRgba(mOnFinishTextColor);
}

bool Command::copyFrom(Command* other)
{
    if (!other)
    {
        LOG_ERROR("No commad specified for copying");
        return false;
    }

    int TODO; // copy command common data
//    DRAKON::IconType mType;
//    QString mText;
//    QString mErrorText;
//    QString mOnStartConsoleText;
//    QString mOnFinishConsoleText;
//    uint32_t mFlags = 0; // Command flags here, by default the command is not interactive
//    uint32_t mOnStartTextColor;
//    uint32_t mOnFinishTextColor;
//    int mExecutionDelay;
//    VariableController* mVarCtrl;
//    SystemState* mSystemState;
//    QList<Command*> mNextCommands;
//    ValencyPoint::Role mRole; // TODO role belongs to valency point not the command
//    bool mHasError;
//    qint64 mID;
//    static qint64 smCounter; // command creation counter in current session

    return loadFromImpl(other);
}

bool Command::loadFromImpl(Command* other)
{
    return true;
}
