#ifndef SHAPEEDITDIALOG_H
#define SHAPEEDITDIALOG_H

#include <QDialog>

class Command;

class ShapeEditDialog : public QDialog
{
    Q_OBJECT

public:
    ShapeEditDialog(QWidget * parent);
    ~ShapeEditDialog();

    void setCommand(Command* command);

protected:

private slots:

private:
};

#endif // SHAPEEDITDIALOG_H
