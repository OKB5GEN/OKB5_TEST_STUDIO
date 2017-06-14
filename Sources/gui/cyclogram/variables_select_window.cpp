#include "Headers/gui/cyclogram/variables_select_window.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

namespace
{
}

VariablesSelectWindow::VariablesSelectWindow(QWidget * parent):
    QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mVariablesList = new QListWidget(this);
    //mVariablesList->setSelectionMode(QAbstractItemView::NoSelection);

    mainLayout->addWidget(mVariablesList);

    QHBoxLayout* checkBoxLayout = new QHBoxLayout();

    mSelectAllBox = new QCheckBox(tr("Select/deselect all"), this);
    checkBoxLayout->addWidget(mSelectAllBox);
    checkBoxLayout->addStretch();
    mainLayout->addLayout(checkBoxLayout);
    connect(mSelectAllBox, SIGNAL(stateChanged(int)), this, SLOT(onSelectAllCheckBoxStateChanged(int)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(mainLayout);
    setWindowTitle(tr("Select variables to show"));

    mSelectAllBox->setEnabled(false);
}

VariablesSelectWindow::~VariablesSelectWindow()
{

}

void VariablesSelectWindow::setCyclogram(QSharedPointer<Cyclogram> cyclogram, const QStringList& excludeVariables)
{
    mSelectedVariables.clear();

    mVariablesList->clear();
    VariableController* controller = cyclogram->variableController();

    auto allVariables = controller->variablesData().keys().toSet();
    auto existingVariables = excludeVariables.toSet();
    auto variblesToAdd = allVariables.subtract(existingVariables);

    foreach (QString key, variblesToAdd)
    {
        QListWidgetItem* item = new QListWidgetItem();
        mVariablesList->addItem(item);
        QCheckBox* checkBox = new QCheckBox(key, mVariablesList);
        connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onSelectVarCheckBoxStateChanged(int)));
        mVariablesList->setItemWidget(item, checkBox);
    }

    mSelectAllBox->setEnabled(mVariablesList->count() > 0);
}

void VariablesSelectWindow::onSelectAllCheckBoxStateChanged(int state)
{
    for(int row = 0; row < mVariablesList->count(); row++)
    {
        QListWidgetItem* item = mVariablesList->item(row);
        QCheckBox* tmp = qobject_cast<QCheckBox*>(mVariablesList->itemWidget(item));
        if (!tmp)
        {
            continue;
        }

        if (tmp->checkState() != state)
        {
            tmp->setCheckState(Qt::CheckState(state));
        }
    }
}

void VariablesSelectWindow::onSelectVarCheckBoxStateChanged(int state)
{
    QCheckBox* changedCheckBox = qobject_cast<QCheckBox*>(QObject::sender());
    QString varName = changedCheckBox->text();

    if (state == Qt::Checked)
    {
        mSelectedVariables.append(varName);
    }
    else
    {
        mSelectedVariables.removeAll(varName);
    }
}

QStringList VariablesSelectWindow::selectedVariables() const
{
    return mSelectedVariables;
}
