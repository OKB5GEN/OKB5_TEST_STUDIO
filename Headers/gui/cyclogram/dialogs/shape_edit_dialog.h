#ifndef SHAPE_EDIT_DIALOG_H
#define SHAPE_EDIT_DIALOG_H

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

#endif // SHAPE_EDIT_DIALOG_H
