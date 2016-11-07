#include "Headers/commandscontroller.h"
#include "Headers/command.h"

CommandsController::CommandsController(QObject * parent):QObject(parent)
{

}

void CommandsController::addCommand(Command* cmd)
{
    connect(cmd, SIGNAL(onFinished(Command*)), this, SLOT(onCommandFinished(Command*)));
}

void CommandsController::clear()
{
    stop();
    m_commandsQueue.clear();
}

void CommandsController::start()
{
    if (!m_isRunning && !m_commandsQueue.empty())
    {
        m_isRunning = true;
        m_commandsQueue.front()->run();
    }
}

void CommandsController::stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        m_commandsQueue.front()->stop();
    }
}

void CommandsController::onCommandFinished(Command* cmd)
{
    if (m_isRunning && cmd == m_commandsQueue.front())
    {
        cmd->deleteLater();
        m_commandsQueue.pop_front();

        if (!m_commandsQueue.empty())
        {
            m_commandsQueue.front()->run();
        }
        else
        {
            m_isRunning = false;
        }
    }
}
