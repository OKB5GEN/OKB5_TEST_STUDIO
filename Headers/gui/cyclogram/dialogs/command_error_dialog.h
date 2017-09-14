#ifndef COMMAND_ERROR_DIALOG_H
#define COMMAND_ERROR_DIALOG_H

#include <QDialog>

class QLabel;

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
