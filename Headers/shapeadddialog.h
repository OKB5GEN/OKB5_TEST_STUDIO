#ifndef SHAPEADDDIALOG_H
#define SHAPEADDDIALOG_H

#include <QDialog>

#include "Headers/shapetypes.h"

class ShapeAddDialog : public QDialog
{
    Q_OBJECT

public:
    ShapeAddDialog(QWidget * parent);
    ~ShapeAddDialog();

    ShapeTypes shapeType() const;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    ShapeTypes mShapeType;

};

#endif // SHAPEADDDIALOG_H
