#include "Headers/logic/commands/cmd_state_start.h"
#include "Headers/logger/Logger.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

CmdStateStart::CmdStateStart(QObject * parent):
    Command(DRAKON::BRANCH_BEGIN, 1, parent)
{
     mFlags = (Command::Selectable | Command::Editable | Command::Deletable);
}

void CmdStateStart::setText(const QString& text)
{
    QString textBefore = mText;
    mText = text;

    if (textBefore != mText)
    {
        emit dataChanged(mText);
    }
}

void CmdStateStart::writeCustomAttributes(QXmlStreamWriter* writer)
{
    writer->writeAttribute("name", mText);
}

void CmdStateStart::readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion)
{
    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("name"))
    {
        mText = attributes.value("name").toString();
    }
}

bool CmdStateStart::loadFromImpl(Command* other)
{
    CmdStateStart* otherStateStartCmd = qobject_cast<CmdStateStart*>(other);
    if (!otherStateStartCmd)
    {
        LOG_ERROR(QString("Command type mismatch (not branch begin)"));
        return false;
    }

    setText(otherStateStartCmd->text());
    return true;
}
