#include "Headers/logic/commands/cmd_title.h"

/* Что надо сделат для реализации переменных?
 *
 * 1. Надо сделать отрисовку квадратика у TITLE
 * 2. Надо сделать отображение переменных в нем
 * 3. В логике наверное надо как-то сделать привязку к переменнной
 * 4. Для этого надо хотя бы математическую Action команду сделать
*/

CmdTitle::CmdTitle(QObject * parent):
    Command(DRAKON::TERMINATOR, 0, parent)
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
