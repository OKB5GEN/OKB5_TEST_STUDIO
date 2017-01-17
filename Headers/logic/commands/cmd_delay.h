#ifndef CMD_DELAY_H
#define CMD_DELAY_H

#include "Headers/logic/command.h"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class CmdDelay: public Command
{
    Q_OBJECT

public:
    CmdDelay(QObject* parent);

    void run() override;
    void stop() override;
    void pause() override;
    void resume() override;

    void setDelay(int hours, int minutes, int seconds, int msec);
    int delay() const; // milliseconds

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;

private slots:
    void finish();

private:
    void setDelay(int msec);

    QTimer* mTimer;
    int mDelay; // msec
    int mTimeLeft;
};
#endif // CMD_DELAY_H
