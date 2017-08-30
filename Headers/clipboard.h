#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QSharedPointer>

class Command;
class Cyclogram;

class Clipboard: public QObject
{
    Q_OBJECT

public:
    Clipboard();
    ~Clipboard();

    Command* commandToCopy() const;
    void setCommandToCopy(Command* command, QSharedPointer<Cyclogram> cyclogram);

private:
    Command* mCommandToCopy;
    QSharedPointer<Cyclogram> mCyclogram; // we store shared pointer instead of weak pointer, because we must always have possibility to create a copy
};
#endif // CLIPBOARD_H
