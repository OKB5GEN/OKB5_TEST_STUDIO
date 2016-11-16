#include "Headers/shapeeditdialog.h"
#include "Headers/sortingbox.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>

ShapeEditDialog::ShapeEditDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mLineEdit = new QLineEdit(this);
    mLineEdit->setText("Default value");
    layout->addWidget(mLineEdit, 0, 0);

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

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

QString ShapeEditDialog::text() const
{
    return mLineEdit->text();
}

void ShapeEditDialog::setText(const QString& text)
{
    mLineEdit->setText(text);
}
