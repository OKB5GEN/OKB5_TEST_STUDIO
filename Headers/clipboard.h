#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QSharedPointer>

class Command;
class Cyclogram;
class SystemState;

class Clipboard: public QObject
{
    Q_OBJECT

public:
    Clipboard();
    ~Clipboard();

    Command* commandToCopy() const;
    Command* createCommandCopy(QSharedPointer<Cyclogram> cyclogram);
    Command* createBranchCopy(QSharedPointer<Cyclogram> cyclogram);

    QSharedPointer<Cyclogram> cyclogram() const;

    void setSystemState(SystemState* state);

    void setCommandToCopy(Command* command, QSharedPointer<Cyclogram> cyclogram);
    static Command* createCommandCopy(Command* from, QSharedPointer<Cyclogram> cyclogram);
    static Command* createBranchCopy(Command* from, QSharedPointer<Cyclogram> cyclogram);

private:
    Command* mCommandToCopy;
    QSharedPointer<Cyclogram> mCyclogram; // we store shared pointer instead of weak pointer, because we must always have possibility to create a copy
    SystemState* mSystemState;
};
#endif // CLIPBOARD_H
