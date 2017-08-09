#ifndef CMD_PARALLEL_PROCESS_H
#define CMD_PARALLEL_PROCESS_H

#include "Headers/logic/command.h"

class CmdParallelProcess: public Command
{
    Q_OBJECT

public:
    CmdParallelProcess(QObject* parent);

    void run() override;
    void stop() override;
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause() override;
    void resume() override;
#endif

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion) override;
    bool loadFromImpl(Command* other) override;

private slots:
    void finish();

private:
    void setDelay(int msec);
};

#endif // CMD_PARALLEL_PROCESS_H

