#ifndef APP_CONSOLE_H
#define APP_CONSOLE_H

#include <QWidget>

class TextEditAppender;

class AppConsole : public QWidget
{
    Q_OBJECT

public:
    AppConsole(QWidget * parent);
    ~AppConsole();

private slots:

private:
    TextEditAppender* mTextEditAppender;
};

#endif // MONITOR_MANUAL_H
