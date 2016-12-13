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
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:


private:
};

#endif // SHAPEEDITDIALOG_H
