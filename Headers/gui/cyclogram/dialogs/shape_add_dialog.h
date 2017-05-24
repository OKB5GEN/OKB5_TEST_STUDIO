#ifndef SHAPE_ADD_DIALOG_H
#define SHAPE_ADD_DIALOG_H

#include <QDialog>
#include <QMap>

#include "Headers/shape_types.h"

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

class ValencyPoint;

class ShapeAddDialog : public QDialog
{
    Q_OBJECT

public:
    ShapeAddDialog(QWidget * parent);
    ~ShapeAddDialog();

    DRAKON::IconType shapeType() const;
    void setValencyPoint(const ValencyPoint& point);

    int param() const;

private slots:
    void onCurrentIndexChanged(int index);

private:
    void setDefaultList();
    bool eventFilter(QObject *obj, QEvent *event) override;

    DRAKON::IconType mShapeType;

    int mCurrentIndex;
    QComboBox* mComboBox;

    int mParam;
};

#endif // SHAPE_ADD_DIALOG_H
