#ifndef CMD_STATE_START_H
#define CMD_STATE_START_H

#include "Headers/logic/command.h"

class CmdStateStart: public Command
{
    Q_OBJECT

public:
    CmdStateStart(QObject * parent);

public slots:
    void setText(const QString& text);

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;
    bool loadFromImpl(Command* other) override;

private:
};

#endif // CMD_STATE_START_H
