#ifndef CYCLOGRAM_CONSOLE_H
#define CYCLOGRAM_CONSOLE_H

#include <QWidget>

class Command;

class QTextEdit;
class QColor;

class CyclogramConsole : public QWidget
{
    Q_OBJECT

public:
    CyclogramConsole(QWidget * parent);
    ~CyclogramConsole();

public slots:
    void onCommandStarted(Command* command);
    void onCommandFinished(Command* command);

private slots:

private:
    void addMessage(const QColor& color, const QString& message);

    QTextEdit* mTextEdit;
};

#endif // CYCLOGRAM_CONSOLE_H
