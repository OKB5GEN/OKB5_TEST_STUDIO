#ifndef SHAPEEDITDIALOG_H
#define SHAPEEDITDIALOG_H

#include <QDialog>

#include "Headers/shapetypes.h"

class ShapeEditDialog : public QDialog
{
    Q_OBJECT

public:
    ShapeEditDialog(QWidget * parent);
    ~ShapeEditDialog();

    //ShapeTypes shapeType() const;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    //ShapeTypes mShapeType;
};

#endif // SHAPEEDITDIALOG_H
