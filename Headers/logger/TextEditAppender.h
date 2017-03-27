#ifndef TEXT_EDIT_APPENDER_H
#define TEXT_EDIT_APPENDER_H

#include "CuteLogger_global.h"
#include "AbstractStringAppender.h"

class QTextEdit;

class TextEditAppender : public AbstractStringAppender
{
public:
    TextEditAppender();
    virtual QString format() const;
    void ignoreEnvironmentPattern(bool ignore);

    void setTextEdit(QTextEdit* textEdit);

protected:
    virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                        const char* function, const QString& category, const QString& message);

private:
    bool m_ignoreEnvPattern;
    QTextEdit* mTextEdit;
};

#endif // TEXT_EDIT_APPENDER_H
