#include "Headers/shapeadddialog.h"
#include "Headers/sortingbox.h"

#include <QDialogButtonBox>
#include <QGridLayout>

ShapeAddDialog::ShapeAddDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 0, 0);

    setLayout(layout);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // TODO Create widget with tabs/filtering to create shapes
    // вероятно это будет прокручиваемый список/таблица из элементов-иконок циклограммы (тут надо подумать над дизайном)
    mShapeType = ShapeTypes::ACTION;
}

ShapeAddDialog::~ShapeAddDialog()
{

}

ShapeTypes ShapeAddDialog::shapeType() const
{
    return mShapeType;
}

void ShapeAddDialog::paintEvent(QPaintEvent *event)
{

}

void ShapeAddDialog::mousePressEvent(QMouseEvent *event)
{

}

void ShapeAddDialog::mouseReleaseEvent(QMouseEvent *event)
{

}

void ShapeAddDialog::mouseMoveEvent(QMouseEvent *event)
{

}
