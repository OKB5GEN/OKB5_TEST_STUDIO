#ifndef CMD_OUTPUT_H
#define CMD_OUTPUT_H

#include "Headers/logic/command.h"

class CmdOutput: public Command
{
    Q_OBJECT

public:
    CmdOutput(QObject* parent);

    void run() override;
    void stop() override;

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;
    bool loadFromImpl(Command* other) override;

private slots:
    void finish();

private:
};
#endif // CMD_DELAY_H
