#include "Headers/gui/cyclogram/dialogs/shape_add_dialog.h"
#include "Headers/gui/cyclogram/cyclogram_widget.h"
#include "Headers/logic/commands/cmd_question.h"
#include "Headers/gui/cyclogram/shape_item.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>

ShapeAddDialog::ShapeAddDialog(QWidget * parent):
    QDialog(parent),
    mParam(-1)
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
    DRAKON::IconType type = item->command()->type();

    switch (type)
    {
    case DRAKON::DELAY:
    case DRAKON::ACTION_MATH:
    case DRAKON::ACTION_MODULE:
    case DRAKON::QUESTION:
        {
            setDefaultList();

            if (point.canBeLanded())
            {
                mComboBox->addItem(tr("Switch state"), QVariant(int(DRAKON::QUESTION)));
            }
        }
        break;
    case DRAKON::BRANCH_BEGIN:
        {
            if (point.role() == ValencyPoint::Down) // add usual command
            {
                setDefaultList();

                if (point.canBeLanded())
                {
                    mComboBox->addItem(tr("Switch state"), QVariant(int(DRAKON::QUESTION)));
                }
            }
            else // add new branch
            {
                mComboBox->addItem(tr("New Branch"), QVariant(int(DRAKON::BRANCH_BEGIN)));
                mComboBox->setCurrentIndex(0);
            }
        }
        break;
/*
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

    if (mShapeType == DRAKON::QUESTION)
    {
        if (mComboBox->itemText(index) == tr("Question"))
        {
            mParam = CmdQuestion::IF;
        }
        else if (mComboBox->itemText(index) == tr("Cycle"))
        {
            mParam = CmdQuestion::CYCLE;
        }
        else if (mComboBox->itemText(index) == tr("Switch state"))
        {
            mParam = CmdQuestion::SWITCH_STATE;
        }
    }
    else
    {
        mParam = -1;
    }
}

 int ShapeAddDialog::param() const
 {
     return mParam;
 }

 void ShapeAddDialog::setDefaultList()
 {
     mComboBox->addItem(tr("Action (Module)"), QVariant(int(DRAKON::ACTION_MODULE)));
     mComboBox->addItem(tr("Action (Math)"), QVariant(int(DRAKON::ACTION_MATH)));
     mComboBox->addItem(tr("Delay"), QVariant(int(DRAKON::DELAY)));
     mComboBox->addItem(tr("Question"), QVariant(int(DRAKON::QUESTION)));
     //mComboBox->addItem(tr("Cycle"), QVariant(int(DRAKON::QUESTION))); // temporarily remove cycles

     //mComboBox->setCurrentIndex(1); //Action module command by deafult as more frequently used
 }
