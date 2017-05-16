#ifndef CMD_OUTPUT_H
#define CMD_OUTPUT_H

#include "Headers/logic/command.h"

//class QTimer;

class CmdOutput: public Command
{
    Q_OBJECT

public:
    CmdOutput(QObject* parent);

    void run() override;
    void stop() override;
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause() override;
    void resume() override;
#endif

//    void setDelay(int hours, int minutes, int seconds, int msec);
//    int delay() const; // milliseconds

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

private slots:
    void finish();

private:
    void setDelay(int msec);

//    QTimer* mTimer;
//    int mDelay; // msec
//    int mTimeLeft;
};
#endif // CMD_DELAY_H
