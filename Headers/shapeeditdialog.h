#ifndef SHAPEEDITDIALOG_H
#define SHAPEEDITDIALOG_H

#include <QDialog>
#include "Headers/shapetypes.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class ShapeEditDialog : public QDialog
{
    Q_OBJECT

public:
    ShapeEditDialog(QWidget * parent);
    ~ShapeEditDialog();

    //ShapeTypes shapeType() const;
    void setText(const QString& text);

    QString text() const;


protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:


private:
    QLineEdit* mLineEdit;
    //ShapeTypes mShapeType;
};

#endif // SHAPEEDITDIALOG_H
