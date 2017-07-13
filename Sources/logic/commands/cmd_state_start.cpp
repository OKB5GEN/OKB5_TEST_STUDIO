#include "Headers/logic/commands/cmd_state_start.h"
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

void CmdStateStart::readCustomAttributes(QXmlStreamReader* reader)
{
    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("name"))
    {
        mText = attributes.value("name").toString();
    }
}
