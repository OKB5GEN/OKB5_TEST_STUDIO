#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

namespace
{
    static const int BTN_SIZE = 32;
    static const qreal PRECISION = 0.001;
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
    list.append(tr("Name"));
    list.append(tr("Default value"));
    list.append(tr("Description"));
    mTableWidget->setColumnCount(list.size());
    mTableWidget->setHorizontalHeaderLabels(list);
    mTableWidget->horizontalHeader()->setStretchLastSection(true);
    mTableWidget->setMinimumSize(600, 400);

    connect(mTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onTableSelectionChanged()));
    connect(mTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onItemChanged(QTableWidgetItem*)));
    connect(mTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onItemDoubleClicked(QTableWidgetItem*)));
    mTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    mainLayout->addWidget(mTableWidget);

    QHBoxLayout* checkBoxLayout = new QHBoxLayout();

    mSelectAllBox = new QCheckBox(tr("Select/deselect all"), this);
    checkBoxLayout->addWidget(mSelectAllBox);
    checkBoxLayout->addStretch();
    mainLayout->addLayout(checkBoxLayout);
    connect(mSelectAllBox, SIGNAL(stateChanged(int)), this, SLOT(onSelectAllCheckBoxStateChanged(int)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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

    VariableController* controller = cyclogram->variableController();

    connect(cyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    foreach (QString key, controller->variablesData().keys())
    {
        int index = mTableWidget->rowCount();
        qreal initial = controller->initialValue(key, -1);
        QString desc = controller->description(key);
        addRow(index, key, initial, desc, true);
    }

    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);
}

void VariablesWindow::onAddClicked()
{
    QString prefix = "NewVar";
    QString name = prefix;
    int index = 0;

    while (isVariableExist(name))
    {
        ++index;
        name = prefix + QString::number(index);
    }

    addRow(mTableWidget->rowCount(), name, 0, "", false);
    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);
}

void VariablesWindow::onRemoveClicked()
{
    QList<int> selectedRows;

    foreach (auto index, mTableWidget->selectionModel()->selectedRows())
    {
        selectedRows.append(index.row());
        QTableWidgetItem* item = mTableWidget->item(index.row(), 0);
        if (!item)
        {
            LOG_ERROR(QString("Invalid item detected! Row: %1, Column: %2").arg(index.row()).arg(0));
            continue;
        }
    }

    int rowsDeleted = 0;
    qSort(selectedRows);
    foreach (int row, selectedRows)
    {
        mTableWidget->removeRow(row - rowsDeleted);
        ++rowsDeleted;
    }

    mSelectAllBox->setCheckState(Qt::Unchecked);
    mSelectAllBox->setEnabled(mTableWidget->rowCount() > 0);

    mRemoveBtn->setEnabled(false);
}

void VariablesWindow::addRow(int row, const QString& name, qreal defaultValue, const QString& description, bool existingVariable)
{
    mTableWidget->insertRow(row);

    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    if (existingVariable)
    {
        nameItem->setData(Qt::UserRole, QVariant(name));
    }

    nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    mTableWidget->setItem(row, 0, nameItem);

    QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(defaultValue));
    valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    mTableWidget->setItem(row, 1, valueItem);

    QTableWidgetItem* descriptionItem = new QTableWidgetItem(description);
    descriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    mTableWidget->setItem(row, 2, descriptionItem);
}

void VariablesWindow::onAccept()
{
    // Here we need to analyze and apply all user actions from GUI to variable controller:

    // 1. Get all existing variables from variable controller
    auto cyclogram = mCyclogram.lock();
    VariableController* controller = cyclogram->variableController();

    // 2. Perform variable rename operation
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QTableWidgetItem* nameItem = mTableWidget->item(row, 0);
        QString varName = nameItem->text();
        QString prevName = nameItem->data(Qt::UserRole).toString();
        if (!prevName.isEmpty() && prevName != varName) // variable existed before, and its name changed
        {
            controller->renameVariable(varName, prevName);
        }
    }

    // 3. For each existing variable:
    //    - Compare existing default value and its table value, if not equal - set new value
    //    - Compare existion description and its table value, if not equal - set new value
    // 4. Add variables that are in table, but not in variable controller

    QSet<QString> newVariables;
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QTableWidgetItem* nameItem = mTableWidget->item(row, 0);
        QTableWidgetItem* valueItem = mTableWidget->item(row, 1);
        QTableWidgetItem* descriptionItem = mTableWidget->item(row, 2);

        QString varName = nameItem->text();
        qreal value = valueItem->text().toDouble();
        QString description = descriptionItem->text();

        newVariables.insert(varName);

        if (controller->isVariableExist(varName))
        {
            if (controller->description(varName) != description)
            {
                controller->setDescription(varName, description);
            }

            if (qAbs(controller->initialValue(varName) - value) > PRECISION)
            {
                controller->setInitialValue(varName, value);
            }
        }
        else
        {
            controller->addVariable(varName, value);
            controller->setDescription(varName, description);
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
    if (state == Qt::Checked)
    {
        mTableWidget->selectAll();
    }
    else
    {
        mTableWidget->clearSelection();
    }
}

bool VariablesWindow::isVariableExist(const QString& name, int excludeRow) const
{
    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QTableWidgetItem* item = mTableWidget->item(row, 0);

        if(!item)
        {
            continue;
        }

        if (excludeRow != -1 && row == excludeRow)
        {
            continue;
        }

        if (item->text() == name)
        {
            return true;
        }
    }

    return false;
}

void VariablesWindow::moveItem(int offset)
{
    int rowFrom = mTableWidget->selectionModel()->selectedRows().at(0).row();

    QTableWidgetItem* nameItem = mTableWidget->item(rowFrom, 0);
    QString variable = nameItem->text();
    QString prevName = nameItem->data(Qt::UserRole).toString();
    qreal value = mTableWidget->item(rowFrom, 1)->text().toDouble();
    QString description = mTableWidget->item(rowFrom, 2)->text();

    mTableWidget->removeRow(rowFrom);

    addRow(rowFrom + offset, variable, value, description, !prevName.isEmpty());
    mTableWidget->selectRow(rowFrom + offset);
}

void VariablesWindow::onMoveUpClicked()
{
    moveItem(-1);
}

void VariablesWindow::onMoveDownClicked()
{
    moveItem(1);
}

void VariablesWindow::onTableSelectionChanged()
{
    int rowsSelected = mTableWidget->selectionModel()->selectedRows().size();

    mRemoveBtn->setEnabled(rowsSelected > 0);
    if (rowsSelected == 1)
    {
        int row = mTableWidget->selectionModel()->selectedRows().at(0).row();
        mMoveUpBtn->setEnabled(row > 0);
        mMoveDownBtn->setEnabled(row < (mTableWidget->rowCount() - 1));
    }
    else
    {
        mMoveUpBtn->setEnabled(false);
        mMoveDownBtn->setEnabled(false);
    }
}

void VariablesWindow::onItemChanged(QTableWidgetItem* item)
{
    switch (item->column())
    {
    case 0: // name changed, check is name unique
        {
            QString newName = item->text();
            if (isVariableExist(newName, item->row()))
            {
                QMessageBox::warning(this, tr("Error"), tr("Variable '%1' already exist").arg(newName));
                item->setText(mPrevText);
            }
        }
        break;

    case 1: // value changed, no special control needed
        {
            bool ok = false;
            item->text().toDouble(&ok);
            if (!ok)
            {
                QMessageBox::warning(this, tr("Error"), tr("Incorrect variable value"));
                item->setText(mPrevText);
            }
        }
        break;
    //case 2: // description changed, no special control needed
    default:
        break;
    }
}

void VariablesWindow::onItemDoubleClicked(QTableWidgetItem* item)
{
    switch (item->column())
    {
    case 0: // name activated
    case 1: // value activated
        {
            mPrevText = item->text();
        }
        break;
    //case 2: // description changed, no special control needed
    default:
        break;
    }
}

void VariablesWindow::keyPressEvent(QKeyEvent *event)
{
    bool processed = true;

    switch (event->key())
    {
        case Qt::Key_Delete:
        {
            if (mRemoveBtn->isEnabled())
            {
                onRemoveClicked();
            }
        }
        break;
    default:
        processed = false;
        break;
    }

    if (!processed)
    {
        QDialog::keyPressEvent(event);
    }
}
