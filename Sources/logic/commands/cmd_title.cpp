#include "Headers/logic/commands/cmd_title.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


CmdTitle::CmdTitle(QObject * parent):
    Command(DRAKON::TERMINATOR, 1, parent)
{
    mFlags = (Selectable | Editable);
}

CmdTitle::TitleType CmdTitle::titleType() const
{
    return mTitleType;
}

void CmdTitle::setTitleType(CmdTitle::TitleType type)
{
    mTitleType = type;

    if (mTitleType == CmdTitle::BEGIN)
    {
        mText = tr("START");
    }
    else
    {
        mText = tr("FINISH");
    }
}

void CmdTitle::writeCustomAttributes(QXmlStreamWriter* writer)
{
    writer->writeAttribute("name", mText);
}

void CmdTitle::readCustomAttributes(QXmlStreamReader* reader)
{
    int TODO_XML;
}

