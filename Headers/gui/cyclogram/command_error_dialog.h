#ifndef COMMAND_ERROR_DIALOG_H
#define COMMAND_ERROR_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class CommandErrorDialog: public QDialog
{
    Q_OBJECT

public:
    CommandErrorDialog(QWidget * parent);
    ~CommandErrorDialog();

    void setText(const QString& text);

private:
    QLabel* mLabel;
};

#endif // COMMAND_ERROR_DIALOG_H
