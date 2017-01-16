#include "Headers/logic/commands/cmd_title.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMetaEnum>

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
    QMetaEnum type = QMetaEnum::fromType<CmdTitle::TitleType>();

    writer->writeAttribute("name", mText);
    writer->writeAttribute("cmd_type", type.valueToKey(mTitleType));
}

void CmdTitle::readCustomAttributes(QXmlStreamReader* reader)
{
    QMetaEnum type = QMetaEnum::fromType<CmdTitle::TitleType>();

    QXmlStreamAttributes attributes = reader->attributes();
    if (attributes.hasAttribute("name"))
    {
        mText = attributes.value("name").toString();
    }

    if (attributes.hasAttribute("cmd_type"))
    {
        QString str = attributes.value("cmd_type").toString();
        mTitleType = CmdTitle::TitleType(type.keyToValue(qPrintable(str)));
    }
}

