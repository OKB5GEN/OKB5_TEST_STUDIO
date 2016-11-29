#ifndef CMD_TITLE_H
#define CMD_TITLE_H

#include "Headers/command.h"

class CmdTitle: public Command
{
    Q_OBJECT

public:
    enum TitleType
    {
        BEGIN,
        END
    };

    CmdTitle(TitleType type, QObject * parent);

    TitleType titleType() const;

private:
    TitleType mTitleType;
};
#endif // CMD_TITLE_H