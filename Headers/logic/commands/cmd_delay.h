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

    void setDelay(const QString& variable);
    void setDelay(int hours, int minutes, int seconds, int msec);
    int delay() const; // milliseconds
    const QString& variable() const; // milliseconds

protected:
    void writeCustomAttributes(QXmlStreamWriter* writer) override;
    void readCustomAttributes(QXmlStreamReader* reader, const Version& fileVersion) override;
    bool loadFromImpl(Command* other) override;
    void onNameChanged(const QString& newName, const QString& oldName) override;
    void onVariableRemoved(const QString& name) override;

private slots:
    void finish();

private:
    void setDelay(int msec);
    void setText(const QString& text);

    QTimer* mTimer;
    int mDelay; // msec
    int mTimeLeft;
    QString mVariable;
};
#endif // CMD_DELAY_H
