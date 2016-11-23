#include "Headers/command.h"

Command::Command(QObject * parent):
    QObject(parent),
    mNext(Q_NULLPTR)
{

}

Command::~Command()
{

}

void Command::run()
{

}

void Command::stop()
{

}

void Command::pause()
{

}

void Command::resume()
{

}

void Command::setNextCommand(Command* cmd)
{
    mNext = cmd;
}
