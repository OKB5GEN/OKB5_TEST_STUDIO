#include "Headers/commands/cmd_title.h"

CmdTitle::CmdTitle(CmdTitle::TitleType type, QObject * parent):
    Command(DRAKON::TERMINATOR, parent),
    mTitleType(type)
{
    if (mTitleType == CmdTitle::BEGIN)
    {
        mText = tr("START");
    }
    else
    {
         mText = tr("FINISH");
    }
}

CmdTitle::TitleType CmdTitle::titleType() const
{
    return mTitleType;
}
