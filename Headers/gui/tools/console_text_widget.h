#ifndef CONSOLE_TEXT_WIDGET_H
#define CONSOLE_TEXT_WIDGET_H

#include <QWidget>

class QComboBox;
class QLineEdit;

class Command;

class ConsoleTextWidget: public QWidget
{
    Q_OBJECT

public:
    ConsoleTextWidget(QWidget * parent);
    ~ConsoleTextWidget();

    void setCommand(Command* command);
    void saveCommand();

public slots:
private slots:

private:
    void setColors(QComboBox* box);

    QComboBox* mStartColor;
    QComboBox* mFinishColor;
    QLineEdit* mStartEdit;
    QLineEdit* mFinishEdit;

    Command* mCommand;
};

#endif // CONSOLE_TEXT_WIDGET_H
