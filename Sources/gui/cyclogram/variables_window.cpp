#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

namespace
{
    static const int BTN_SIZE = 32;
    static const qreal PRECISION = 0.001;
    static const char* PREV_NAME_PROPERTY = "Prev";
}

VariablesWindow::VariablesWindow(QWidget * parent):
    QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // add button
    mAddBtn = new QToolButton(this);
    mAddBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mAddBtn->setIcon(QIcon(":/resources/images/edit_add.png"));
    mAddBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mAddBtn->setToolTip(tr("Add new variable"));
    connect(mAddBtn, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    buttonLayout->addWidget(mAddBtn);

    // remove button
    mRemoveBtn = new QToolButton(this);
    mRemoveBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setIcon(QIcon(":/resources/images/edit_remove.png"));
    mRemoveBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setToolTip(tr("Remove selected variables"));
    mRemoveBtn->setEnabled(false);
    connect(mRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemoveClicked()));
    buttonLayout->addWidget(mRemoveBtn);

    // move up button
    mMoveUpBtn = new QToolButton(this);
    mMoveUpBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveUpBtn->setIcon(QIcon(":/resources/images/arrow_up.png"));
    mMoveUpBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveUpBtn->setToolTip(tr("Move selected variable up"));
    mMoveUpBtn->setEnabled(false);
    connect(mMoveUpBtn, SIGNAL(clicked()), this, SLOT(onMoveUpClicked()));
    buttonLayout->addWidget(mMoveUpBtn);

    // move down button
    mMoveDownBtn = new QToolButton(this);
    mMoveDownBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveDownBtn->setIcon(QIcon(":/resources/images/arrow_down.png"));
    mMoveDownBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveDownBtn->setToolTip(tr("Move selected variable down"));
    mMoveDownBtn->setEnabled(false);
    connect(mMoveDownBtn, SIGNAL(clicked()), this, SLOT(onMoveDownClicked()));
    buttonLayout->addWidget(mMoveDownBtn);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    mTableWidget = new QTableWidget(this);
    QStringList list;
    list.append("");
    list.append(tr("Name"));
    list.append(tr("Default value"));
    list.append(tr("Description"));
    mTableWidget->setColumnCount(list.size());
    mTableWidget->setHorizontalHeaderLabels(list);
    mTableWidget->horizontalHeader()->setStretchLastSection(true);
    mTableWidget->setMinimumSize(600, 400);
    mTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mTableWidget->setColumnWidth(0, 30);

    mainLayout->addWidget(mTableWidget);

    QHBoxLayout* checkBoxLayout = new QHBoxLayout();

    mSelectAllBox = new QCheckBox(tr("Select/deselect all"), this);
    //mShowAllBox = new QCheckBox(tr("Show all variables"), this);
    checkBoxLayout->addWidget(mSelectAllBox);
    //checkBoxLayout->addWidget(mShowAllBox);
    checkBoxLayout->addStretch();
    mainLayout->addLayout(checkBoxLayout);
    connect(mSelectAllBox, SIGNAL(stateChanged(int)), this, SLOT(onSelectAllCheckBoxStateChanged(int)));
    //connect(mShowAllBox, SIGNAL(stateChanged(int)), this, SLOT(onShowAllCheckBoxStateChanged(int)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mValidator = new QDoubleValidator(this);

    setLayout(mainLayout);
    setWindowTitle(tr("Variables"));

    mSelectAllBox->setEnabled(false);
}

VariablesWindow::~VariablesWindow()
{

}

void VariablesWindow::setCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    mCyclogram = cyclogram;

    mTableWidget->clearContents();
    mTableWidget->setRowCount(0);
    mRenameLog.clear();
    mSelectedRows.clear();

    VariableController* controller = cyclogram->variableController();

    connect(cyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    foreach (QString key, controller->variablesData().keys())
    {
        int index = mTableWidget->rowCount();
        qreal initial = controller->initialValue(key, -1);
        QString desc = controller->description(key);
        addRow(index, key, initial, desc);
    }

    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);
}

void VariablesWindow::onAddClicked()
{
    QString prefix = "NewVar";
    QString name = prefix;
    int index = 0;

    while (isVariableExist(name, Q_NULLPTR))
    {
        ++index;
        name = prefix + QString::number(index);
    }

    addRow(mTableWidget->rowCount(), name, 0, "");
    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);
}

void VariablesWindow::onRemoveClicked()
{
    int rowsDeleted = 0;
    auto sortedDeletionList = mSelectedRows.toList();
    qSort(sortedDeletionList);
    foreach (int row, sortedDeletionList)
    {
        mTableWidget->removeRow(row - rowsDeleted);
        ++rowsDeleted;
    }

    mSelectedRows.clear();
    mSelectAllBox->setCheckState(Qt::Unchecked);
    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);

    mRemoveBtn->setEnabled(false);
}

void VariablesWindow::onNameChanged()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(QObject::sender());

    QString oldName = lineEdit->property(PREV_NAME_PROPERTY).toString();
    QString newName = lineEdit->text();
    if (newName.isEmpty())
    {
        lineEdit->setText(oldName);
        return;
    }

    if (newName == oldName)
    {
        return; // name doesn't changed
    }

    if (isVariableExist(newName, lineEdit))
    {
        // name is not unique, restore old name
        QMessageBox::warning(this, tr("Error"), tr("Variable '%1' already exist").arg(newName));
        lineEdit->setText(oldName);
        return;
    }

    mRenameLog.append(QPair<QString, QString>(oldName, newName));
    lineEdit->setProperty(PREV_NAME_PROPERTY, QVariant(newName));
}

void VariablesWindow::addRow(int row, const QString& name, qreal defaultValue, const QString& description)
{
    mTableWidget->insertRow(row);

    QCheckBox* selectCheckbox = new QCheckBox(mTableWidget);
    mTableWidget->setCellWidget(row, 0, selectCheckbox);
    connect(selectCheckbox, SIGNAL(stateChanged(int)), this, SLOT(onSelectVarCheckBoxStateChanged(int)));

    QLineEdit* lineEditName = new QLineEdit(mTableWidget);
    lineEditName->setFrame(false);
    lineEditName->setText(name);
    lineEditName->setProperty(PREV_NAME_PROPERTY, QVariant(name));
    mTableWidget->setCellWidget(row, 1, lineEditName);
    connect(lineEditName, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));

    QString initial = QString::number(defaultValue);
    QLineEdit* lineEditInitial = new QLineEdit(mTableWidget);
    lineEditInitial->setFrame(false);
    lineEditInitial->setText(initial);
    lineEditInitial->setValidator(mValidator);
    mTableWidget->setCellWidget(row, 2, lineEditInitial);

    QLineEdit* lineEditDescription = new QLineEdit(mTableWidget);
    lineEditDescription->setText(description);
    lineEditDescription->setFrame(false);
    mTableWidget->setCellWidget(row, 3, lineEditDescription);
}

void VariablesWindow::onAccept()
{
    mSelectedVariables.clear();

    // Here we need to analyze and apply all user actions from GUI to variable controller:

    // 1. Get all existing variables from variable controller
    auto cyclogram = mCyclogram.lock();
    VariableController* controller = cyclogram->variableController();

    // 2. Apply rename log to them
    optimizeRenameLog();
    foreach (auto operation, mRenameLog)
    {
        controller->renameVariable(operation.second, operation.first);
    }

    // 3. For each existing variable:
    //    - Compare existing default value and its table value, if not equal - set new value
    //    - Compare existion description and its table value, if not equal - set new value
    // 4. Add variables that are in table, but not in variable controller
    QSet<QString> newVariables;
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QCheckBox* selectedBox = qobject_cast<QCheckBox*>(mTableWidget->cellWidget(row, 0));
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 1));
        QLineEdit* defaultValueEdit = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 2));
        QLineEdit* descriptionEdit = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 3));

        QString varName = nameEdit->text();
        qreal defaultValue = defaultValueEdit->text().replace(",", ".").toDouble();
        QString description = descriptionEdit->text();

        newVariables.insert(varName);

        if (controller->isVariableExist(varName))
        {
            if (controller->description(varName) != description)
            {
                controller->setDescription(varName, description);
            }

            if (qAbs(controller->initialValue(varName) - defaultValue) > PRECISION)
            {
                controller->setInitialValue(varName, defaultValue);
            }
        }
        else
        {
            controller->addVariable(varName, defaultValue);
            controller->setDescription(varName, description);
        }

        if (selectedBox->checkState() == Qt::Checked)
        {
            mSelectedVariables.append(varName);
        }
    }

    // 5. Delete existing variables that are not in table
    auto existingVariables = controller->variablesData().keys().toSet();
    auto variablesToDelete = existingVariables.subtract(newVariables);

    foreach (QString var, variablesToDelete)
    {
        controller->removeVariable(var);
    }

    accept();
}

void VariablesWindow::onSelectAllCheckBoxStateChanged(int state)
{
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QCheckBox* tmp = qobject_cast<QCheckBox*>(mTableWidget->cellWidget(row, 0));
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

void VariablesWindow::onShowAllCheckBoxStateChanged(int state)
{
    LOG_DEBUG(QString("Show all checkbox state changed to %1").arg(state));
}

void VariablesWindow::onSelectVarCheckBoxStateChanged(int state)
{
    QCheckBox* changedCheckBox = qobject_cast<QCheckBox*>(QObject::sender());

    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QCheckBox* tmp = qobject_cast<QCheckBox*>(mTableWidget->cellWidget(row, 0));
        if(tmp != changedCheckBox)
        {
            continue;
        }

        if (state == Qt::Checked)
        {
            mSelectedRows.insert(row);
        }
        else
        {
            mSelectedRows.remove(row);
        }

        break;
    }

    mRemoveBtn->setEnabled(!mSelectedRows.empty());
}

bool VariablesWindow::isVariableExist(const QString& name, QLineEdit* excludeRow) const
{
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QLineEdit* tmp = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 1));
        if(!tmp)
        {
            continue;
        }

        if (excludeRow && tmp == excludeRow)
        {
            continue;
        }

        if (tmp->text() == name)
        {
            return true;
        }
    }

    return false;
}

void VariablesWindow::optimizeRenameLog()
{
    QList<QPair<QString, QString>> optimizedLog;
    while (!mRenameLog.isEmpty())
    {
        auto iter = mRenameLog.begin();
        QString from = (*iter).first;
        QString to = (*iter).second;

        mRenameLog.erase(iter);

        for (auto it = mRenameLog.begin(); it != mRenameLog.end();)
        {
            if ((*it).first == to)
            {
                to = (*it).second;
                it = mRenameLog.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (from != to)
        {
            optimizedLog.append(QPair<QString, QString>(from, to));
        }
    }

    optimizedLog.swap(mRenameLog);
}

QStringList VariablesWindow::selectedVariables() const
{
    return mSelectedVariables;
}

void VariablesWindow::onMoveUpClicked()
{

}

void VariablesWindow::onMoveDownClicked()
{

}
