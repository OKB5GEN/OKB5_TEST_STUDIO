#include "Headers/commands/cmd_title.h"

CmdTitle::CmdTitle(QObject * parent):
    Command(DRAKON::TERMINATOR, parent)
{
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
