#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QSize>

class Command: public QObject
{
    Q_OBJECT

public:
    Command(QObject * parent);
    virtual ~Command();
    virtual void run();
    virtual void stop();

    const QSize& getSize() const { return mSize; }

signals:
    void onFinished(Command* nextCmd); // must be sent on command finish

protected:
    QSize mSize; // must be initialized in command

private:
};

#endif // COMMAND_H
