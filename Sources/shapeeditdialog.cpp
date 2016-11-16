#include "Headers/shapeeditdialog.h"
#include "Headers/sortingbox.h"

#include <QDialogButtonBox>
#include <QGridLayout>

ShapeEditDialog::ShapeEditDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 0, 0);

    setLayout(layout);
    setWindowTitle("Shape Edit Dialog");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    //mShapeType = ShapeTypes::ACTION;
}

ShapeEditDialog::~ShapeEditDialog()
{

}
/*
ShapeTypes ShapeEditDialog::shapeType() const
{
    return mShapeType;
}
*/
void ShapeEditDialog::paintEvent(QPaintEvent *event)
{

}

void ShapeEditDialog::mousePressEvent(QMouseEvent *event)
{

}

void ShapeEditDialog::mouseReleaseEvent(QMouseEvent *event)
{

}

void ShapeEditDialog::mouseMoveEvent(QMouseEvent *event)
{

}
