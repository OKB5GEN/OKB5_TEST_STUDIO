// Local
#include "Headers/logger/TextEditAppender.h"

// STL
#include <iostream>
#include <QTextEdit>

/**
 * \class TextEditAppender
 *
 * \brief TextEditAppender is the simple appender that writes the log records to the QTextEdit.
 *
 * TextEditAppender uses "[%{type:-7}] <%{function}> %{message}\n" as a default output format. It is similar to the
 * AbstractStringAppender but doesn't show a timestamp.
 *
 * You can modify TextEditAppender output format without modifying your code by using \c QT_MESSAGE_PATTERN environment
 * variable. If you need your application to ignore this environment variable you can call
 * ConsoleAppender::ignoreEnvironmentPattern(true)
 */


TextEditAppender::TextEditAppender()
  : AbstractStringAppender(),
    m_ignoreEnvPattern(false),
    mTextEdit(Q_NULLPTR)
{
    setFormat("[%{type:-7}] <%{function}> %{message}\n");
}


QString TextEditAppender::format() const
{
    const QString envPattern = QString::fromLocal8Bit(qgetenv("QT_MESSAGE_PATTERN"));
    return (m_ignoreEnvPattern || envPattern.isEmpty()) ? AbstractStringAppender::format() : (envPattern + "\n");
}


void TextEditAppender::ignoreEnvironmentPattern(bool ignore)
{
    m_ignoreEnvPattern = ignore;
}


//! Writes the log record to the QTextEdit depending on log level.
/**
 * \sa AbstractStringAppender::format()
 */
void TextEditAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                             const char* function, const QString& category, const QString& message)
{
    if (!mTextEdit)
    {
        return;
    }

    uint32_t color;

    switch (logLevel)
    {
    case Logger::Trace:   // Trace level. Can be used for mostly unneeded records used for internal code tracing.
        color = 0xff00b3d6; // light blue
        break;
    case Logger::Debug:   // Debug level. Useful for non-necessary records used for the debugging of the software.
        color = 0xff47d600; // green
        break;
    case Logger::Info:    // Info level. Can be used for informational records, which may be interesting for not only developers.
        color = 0xff000000; // black
        break;
    case Logger::Warning: // Warning. May be used to log some non-fatal warnings detected by your application.
        color = 0xffff6633; // orange
        break;
    case Logger::Error:   // Error. May be used for a big problems making your application work wrong but not crashing.
        color = 0xfff5003d; // red
        break;
    case Logger::Fatal:
        color = 0xffcc33ff; // magenta
        break;
    default:
        color = 0xff000000; // black by default
        break;
    }

    mTextEdit->setTextColor(QColor::fromRgba(color));
    //QPalette p = mTextEdit->palette();
    //p.setColor(QPalette::WindowText, );
    //mTextEdit->setPalette(p);

    mTextEdit->append(formattedString(timeStamp, logLevel, file, line, function, category, message));
    //std::cerr << qPrintable(formattedString(timeStamp, logLevel, file, line, function, category, message));
}

void TextEditAppender::setTextEdit(QTextEdit* textEdit)
{
    mTextEdit = textEdit;

    QFont font;
    font.setPointSize(10);
    font.setFamily("Verdana");
    mTextEdit->setFont(font);
}
