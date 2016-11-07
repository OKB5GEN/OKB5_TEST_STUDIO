#ifndef COMMANDSCONTROLLER_H
#define COMMANDSCONTROLLER_H

#include <QObject>
#include <QList>

class Command;

class CommandsController: public QObject
{
    Q_OBJECT

public:
    CommandsController(QObject * parent);
    void addCommand(Command* cmd);
    void clear();
    void start();
    void stop();

private slots:
    void onCommandFinished(Command* cmd);

private:
    QList<Command*> m_commandsQueue;
    bool m_isRunning = false;
};
#endif // COMMANDSCONTROLLER_H
