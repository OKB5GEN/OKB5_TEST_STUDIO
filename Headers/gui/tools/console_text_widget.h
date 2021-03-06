#ifndef CONSOLE_TEXT_WIDGET_H
#define CONSOLE_TEXT_WIDGET_H

#include <QGroupBox>

class QComboBox;
class QLineEdit;

class Command;

class ConsoleTextWidget: public QGroupBox
{
    Q_OBJECT

public:
    ConsoleTextWidget(QWidget * parent);
    ~ConsoleTextWidget();

    static const QStringList& colorsList();

    void setCommand(Command* command);
    void saveCommand();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setColors(QComboBox* box);

    QComboBox* mColorBox;
    QLineEdit* mTextEdit;

    Command* mCommand;
};

#endif // CONSOLE_TEXT_WIDGET_H
