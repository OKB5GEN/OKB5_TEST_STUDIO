#ifndef CONSOLE_TEXT_WIDGET_H
#define CONSOLE_TEXT_WIDGET_H

#include <QWidget>

class QComboBox;
class QLineEdit;

class ConsoleTextWidget: public QWidget
{
    Q_OBJECT

public:
    ConsoleTextWidget(QWidget * parent);
    ~ConsoleTextWidget();

public slots:
private slots:

private:
    void setColors(QComboBox* box);

    QComboBox* mStartColor;
    QComboBox* mFinishColor;
    QLineEdit* mStartEdit;
    QLineEdit* mFinishEdit;
};

#endif // CONSOLE_TEXT_WIDGET_H
