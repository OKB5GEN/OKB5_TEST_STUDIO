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
    mText = text;
    emit textChanged(mText);
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
