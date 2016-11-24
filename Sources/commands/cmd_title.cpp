#include "Headers/commands/cmd_title.h"

CmdTitle::CmdTitle(CmdTitle::TitleType type, QObject * parent):
    Command(ShapeTypes::TITLE, parent),
    mTitleType(type)
{
}

CmdTitle::TitleType CmdTitle::titleType() const
{
    return mTitleType;
}
