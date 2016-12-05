#include "Headers/shape_edit_dialog.h"
#include "Headers/cyclogram_widget.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>

ShapeEditDialog::ShapeEditDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    int TODO; // remove line edit its just for testing
    QLineEdit* mLineEdit = new QLineEdit(this);
    mLineEdit->setText("Default value");
    layout->addWidget(mLineEdit, 0, 0);

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle("Default Edit Dialog");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

ShapeEditDialog::~ShapeEditDialog()
{

}

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

void ShapeEditDialog::setCommand(Command* command)
{
    int TODO; // create GUI for command params editing
}
