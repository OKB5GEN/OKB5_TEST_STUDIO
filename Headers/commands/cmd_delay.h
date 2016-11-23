#ifndef CMD_DELAY_H
#define CMD_DELAY_H

#include "Headers/command.h"

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

    void setDelay(int seconds);

private slots:
    void finish();

private:
    QTimer* mTimer;
    int mDelay; // msec
};
#endif // CMD_DELAY_H
