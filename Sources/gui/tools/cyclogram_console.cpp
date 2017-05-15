#include "Headers/gui/tools/cyclogram_console.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/command.h"

#include <QtWidgets>

CyclogramConsole::CyclogramConsole(QWidget * parent):
    QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    QPushButton* clearConsoleBtn = new QPushButton(QIcon(":/images/delete_all"), tr("Clear"), this);

    QHBoxLayout* hLayout = new QHBoxLayout();
    layout->addLayout(hLayout, 0, 0);
    hLayout->addWidget(clearConsoleBtn);
    hLayout->addStretch();

    mTextEdit = new QTextEdit(this);
    mTextEdit->setReadOnly(true);
    layout->addWidget(mTextEdit, 1, 0);

    connect(clearConsoleBtn, SIGNAL(clicked()), mTextEdit, SLOT(clear()));
}

CyclogramConsole::~CyclogramConsole()
{

}

void CyclogramConsole::onCommandStarted(Command* command)
{
//    uint32_t color;

//    uint32_t logLevel = Logger::Debug;
//    switch (logLevel)
//    {
//    case Logger::Trace:   // Trace level. Can be used for mostly unneeded records used for internal code tracing.
//        color = 0xff00b3d6; // light blue
//        break;
//    case Logger::Debug:   // Debug level. Useful for non-necessary records used for the debugging of the software.
//        color = 0xff47d600; // green
//        break;
//    case Logger::Info:    // Info level. Can be used for informational records, which may be interesting for not only developers.
//        color = 0xff000000; // black
//        break;
//    case Logger::Warning: // Warning. May be used to log some non-fatal warnings detected by your application.
//        color = 0xffff6633; // orange
//        break;
//    case Logger::Error:   // Error. May be used for a big problems making your application work wrong but not crashing.
//        color = 0xfff5003d; // red
//        break;
//    case Logger::Fatal:
//        color = 0xffcc33ff; // magenta
//        break;
//    default:
//        color = 0xff000000; // black by default
//        break;
//    }

    if (command->onStartConsoleText().isEmpty())
    {
        return;
    }

    QString message = command->onStartConsoleText();
    //TODO parse text to find variable links

    addMessage(command->onStartConsoleTextColor(), message);
}

void CyclogramConsole::onCommandFinished(Command* command)
{
//    uint32_t color;

//    uint32_t logLevel = Logger::Error;
//    switch (logLevel)
//    {
//    case Logger::Trace:   // Trace level. Can be used for mostly unneeded records used for internal code tracing.
//        color = 0xff00b3d6; // light blue
//        break;
//    case Logger::Debug:   // Debug level. Useful for non-necessary records used for the debugging of the software.
//        color = 0xff47d600; // green
//        break;
//    case Logger::Info:    // Info level. Can be used for informational records, which may be interesting for not only developers.
//        color = 0xff000000; // black
//        break;
//    case Logger::Warning: // Warning. May be used to log some non-fatal warnings detected by your application.
//        color = 0xffff6633; // orange
//        break;
//    case Logger::Error:   // Error. May be used for a big problems making your application work wrong but not crashing.
//        color = 0xfff5003d; // red
//        break;
//    case Logger::Fatal:
//        color = 0xffcc33ff; // magenta
//        break;
//    default:
//        color = 0xff000000; // black by default
//        break;
//    }

    if (command->onFinishConsoleText().isEmpty())
    {
        return;
    }

    QString message = command->onFinishConsoleText();
    //TODO parse text to find variable links

    addMessage(command->onFinishConsoleTextColor(), message);
}

void CyclogramConsole::addMessage(const QColor& color, const QString& message)
{
    mTextEdit->setTextColor(color);

    QDateTime timeStamp = QDateTime::currentDateTimeUtc();
    QString msg = timeStamp.toString(QLatin1String("HH:mm:ss.zzz"));
    msg += ": ";
    msg += message;

    mTextEdit->append(msg);
}
