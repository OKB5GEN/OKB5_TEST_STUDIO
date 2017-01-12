#ifndef CMD_TITLE_H
#define CMD_TITLE_H

#include "Headers/logic/command.h"

class CmdTitle: public Command
{
    Q_OBJECT

public:
    enum TitleType
    {
        BEGIN,
        END
    };

    CmdTitle(QObject * parent);

    void setTitleType(TitleType type);
    TitleType titleType() const;

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

private:
    TitleType mTitleType;
};
#endif // CMD_TITLE_H
