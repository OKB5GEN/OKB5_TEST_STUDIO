#include "Headers/clipboard.h"
#include "Headers/logic/cyclogram.h"

Clipboard::Clipboard():
    QObject(Q_NULLPTR),
    mCommandToCopy(Q_NULLPTR)
{

}

Clipboard::~Clipboard()
{

}

Command* Clipboard::commandToCopy() const
{
    return mCommandToCopy;
}

void Clipboard::setCommandToCopy(Command* command, QSharedPointer<Cyclogram> cyclogram)
{
    // 1. Delete existing command copy
    // 2. Create new command copy

    mCyclogram = cyclogram;
    mCommandToCopy = command;
}
