#include "Headers/shape_add_dialog.h"
#include "Headers/cyclogram_widget.h"
#include "Headers/command.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>

ShapeAddDialog::ShapeAddDialog(QWidget * parent):
    QDialog(parent)
{
    QGridLayout * layout = new QGridLayout(this);

    mComboBox = new QComboBox(this);
    layout->addWidget(mComboBox, 0, 0);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    setLayout(layout);
    setWindowTitle(tr("Add Command"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(mComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    setFixedSize(sizeHint());
}

ShapeAddDialog::~ShapeAddDialog()
{

}

DRAKON::IconType ShapeAddDialog::shapeType() const
{
    return mShapeType;
}

void ShapeAddDialog::setValencyPoint(const ValencyPoint& point)
{
    int TODO; // implement switch for all shape types
    mComboBox->clear();

    ShapeItem* item = point.owner();

    switch (item->command()->type())
    {
    case DRAKON::DELAY:
    case DRAKON::ACTION_MATH:
    //case DRAKON::ACTION_MODULE:
        {
            mComboBox->addItem(tr("Delay"), QVariant(int(DRAKON::DELAY)));
            mComboBox->addItem(tr("Action (Math)"), QVariant(int(DRAKON::ACTION_MATH)));
            //mComboBox->addItem(tr("Action (Module)"), QVariant(int(DRAKON::ACTION_MODULE)));

            mComboBox->setCurrentIndex(1); //Action command by deafult as more frequently used
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            int TODO2; // check valency point data to add branch or some other item
            if (point.role() == 0) // add usual command
            {
                mComboBox->addItem(tr("Delay"), QVariant(int(DRAKON::DELAY)));
                mComboBox->addItem(tr("Action (Math)"), QVariant(int(DRAKON::ACTION_MATH)));
                //mComboBox->addItem(tr("Action (Math)"), QVariant(int(DRAKON::ACTION_MODULE)));

                mComboBox->setCurrentIndex(1); //Action command by deafult as more frequently used
            }
            else // add new branch
            {
                mComboBox->addItem(tr("New Branch"), QVariant(int(DRAKON::BRANCH_BEGIN)));

                mComboBox->setCurrentIndex(0); //Action command by deafult as more frequently used
            }
        }
        break;
/*
    case DRAKON::QUESTION:
    case DRAKON::SWITCH:
    case DRAKON::CASE:
    case DRAKON::SUBPROGRAM:
    case DRAKON::FOR_BEGIN:
    case DRAKON::FOR_END:
    case DRAKON::OUTPUT:
    case DRAKON::INPUT:
    case DRAKON::START_TIMER:
    case DRAKON::SYNCHRONIZER:
    case DRAKON::PARALLEL_PROCESS:
    case DRAKON::PARAMS:
    case DRAKON::SHELF:
        break;
        */
    default:
        break;
    }
}

void ShapeAddDialog::onCurrentIndexChanged(int index)
{
    int shape = mComboBox->itemData(index).toInt();
    mShapeType = DRAKON::IconType(shape);
}
