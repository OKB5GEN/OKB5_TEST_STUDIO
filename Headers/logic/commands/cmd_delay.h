#ifndef CMD_DELAY_H
#define CMD_DELAY_H

#include "Headers/logic/command.h"

class QTimer;

class CmdDelay: public Command
{
    Q_OBJECT

public:
    CmdDelay(QObject* parent);

    void run() override;
    void stop() override;
#ifdef ENABLE_CYCLOGRAM_PAUSE
    void pause() override;
    void resume() override;
#endif

    void setDelay(int hours, int minutes, int seconds, int msec);
    int delay() const; // milliseconds

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader) override;
    bool loadFromImpl(Command* other) override;

private slots:
    void finish();

private:
    void setDelay(int msec);
    void setText(const QString& text);

    QTimer* mTimer;
    int mDelay; // msec
    int mTimeLeft;
};
#endif // CMD_DELAY_H
