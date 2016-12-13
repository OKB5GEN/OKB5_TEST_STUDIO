#include "Headers/gui/cyclogram/variables_window.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"

#include <QtWidgets>

namespace
{
    static const int BTN_SIZE = 32;
    static const char* PREV_NAME_PROPERTY = "Prev";
}

VariablesWindow::VariablesWindow(QWidget * parent):
    QDialog(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    mTableWidget = new QTableWidget(this);
    mTableWidget->setColumnCount(3);
    QStringList list;
    list.append(tr("Name"));
    list.append(tr("Initial"));
    list.append(tr("Current"));
    mTableWidget->setHorizontalHeaderLabels(list);

    mTableWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //mTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(mTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onTableSelectionChanged()));
    updateTableSize();

    layout->addWidget(mTableWidget, 0, 0, 5, 5);

    // play button
    QToolButton* addBtn = new QToolButton(this);
    addBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    addBtn->setIcon(QIcon(":/images/edit_add.png"));
    addBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    connect(addBtn, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    layout->addWidget(addBtn, 0, 5, 1, 1);

    // pause button
    mRemoveBtn = new QToolButton(this);
    mRemoveBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setIcon(QIcon(":/images/edit_remove.png"));
    mRemoveBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setEnabled(false);
    connect(mRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemoveClicked()));
    layout->addWidget(mRemoveBtn, 1, 5, 1, 1);

    mValidator = new QDoubleValidator(this);

    setLayout(layout);
    setWindowTitle(tr("Variables"));
}

VariablesWindow::~VariablesWindow()
{

}

void VariablesWindow::setCyclogram(Cyclogram * cyclogram)
{
    mCyclogram = cyclogram;

    mTableWidget->clearContents();
    mTableWidget->setRowCount(0);

    VariableController* controller = mCyclogram->varCtrl();

    connect(controller, SIGNAL(valueChanged(const QString&,qreal,int)), this, SLOT(onValueChanged(const QString&,qreal,int)));

    foreach (QString key, controller->variables().keys())
    {
        int index = mTableWidget->rowCount();
        qreal initial = controller->variable(key, -1, VariableController::Initial);
        qreal current = controller->variable(key, -1, VariableController::Current);
        addRow(index, key, initial, current);
    }

    updateTableSize();
}

void VariablesWindow::onAddClicked()
{
    QString prefix = "V";
    QString name = prefix;
    int index = 0;
    VariableController* controller = mCyclogram->varCtrl();

    while (controller->isVariableExist(name))
    {
        ++index;
        name = prefix + QString::number(index);
    }

    addRow(mTableWidget->rowCount(), name, 0, 0);
    controller->addVariable(name, 0);
    updateTableSize();
}

void VariablesWindow::updateTableSize()
{
    mTableWidget->resizeColumnsToContents();
    mTableWidget->setFixedWidth(mTableWidget->horizontalHeader()->length() + mTableWidget->verticalHeader()->width());
    mTableWidget->update();
}

void VariablesWindow::onRemoveClicked()
{
    VariableController* controller = mCyclogram->varCtrl();
    QItemSelectionModel* selectionModel = mTableWidget->selectionModel();

    QList<int> rowsForDeletion;
    foreach (QModelIndex index, selectionModel->selectedRows())
    {
        rowsForDeletion.append(index.row());
    }

    qSort(rowsForDeletion);
    int rowsDeleted = 0;

    foreach (int row, rowsForDeletion)
    {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row - rowsDeleted, 0));
        if (lineEdit)
        {
            controller->removeVariable(lineEdit->text());
            mTableWidget->removeRow(row - rowsDeleted);
            ++rowsDeleted;
        }
    }

    mTableWidget->clearSelection();
    updateTableSize();
}

void VariablesWindow::onNameChanged()
{
    VariableController* controller = mCyclogram->varCtrl();
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(QObject::sender());
    if (!lineEdit)
    {
        qDebug("WTF 1?");
        return;
    }

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

    if (controller->isVariableExist(newName))
    {
        // name is not unique, restore old name
        QMessageBox::warning(this, tr("Error"), tr("Variable '%1' already exist").arg(newName));
        lineEdit->setText(oldName);
        return;
    }

    lineEdit->setProperty(PREV_NAME_PROPERTY, QVariant(newName));
    controller->renameVariable(newName, oldName);
}

void VariablesWindow::onInitialValueChanged()
{
    VariableController* controller = mCyclogram->varCtrl();
    QLineEdit* valueLineEdit = qobject_cast<QLineEdit*>(QObject::sender());
    if (!valueLineEdit)
    {
        qDebug("WTF 2?");
        return;
    }

    qreal value = valueLineEdit->text().replace(",", ".").toDouble();

    // find out variable name
    QString name;

    for(int row = 0; row < mTableWidget->rowCount(); row++)
    {
        QLineEdit* tmp = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 1));
        if(tmp == valueLineEdit)
        {
            tmp = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 0));
            if (tmp)
            {
                name = tmp->text();
            }
            break;
        }
    }

    controller->setVariable(name, value, VariableController::Initial);
}

void VariablesWindow::onValueChanged(const QString& name, qreal value, int container)
{
    if (container == VariableController::Current)
    {
        for(int row = 0; row < mTableWidget->rowCount(); row++)
        {
            // find line edit with name text in first column
            QLineEdit* tmp = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 0));
            if(tmp && tmp->text() == name)
            {
                // get corresponding current value
                tmp = qobject_cast<QLineEdit*>(mTableWidget->cellWidget(row, 2));
                if (tmp)
                {
                    tmp->setText(QString::number(value));
                }
                break;
            }
        }
    }
}

void VariablesWindow::onTableSelectionChanged()
{
    QItemSelectionModel* selectionModel = mTableWidget->selectionModel();
    int count = selectionModel->selectedRows().size();
    mRemoveBtn->setEnabled(count > 0 && (mCyclogram->state() != Cyclogram::RUNNING));
}

void VariablesWindow::addRow(int row, const QString& name, qreal initialValue, qreal currentValue)
{
    mTableWidget->insertRow(row);

    QLineEdit* lineEditName = new QLineEdit();
    lineEditName->setText(name);
    lineEditName->setProperty(PREV_NAME_PROPERTY, QVariant(name));
    mTableWidget->setCellWidget(row, 0, lineEditName);
    connect(lineEditName, SIGNAL(editingFinished()), this, SLOT(onNameChanged()));

    QString initial = QString::number(initialValue);
    QLineEdit* lineEditInitial = new QLineEdit();
    lineEditInitial->setText(initial);
    lineEditInitial->setValidator(mValidator);
    mTableWidget->setCellWidget(row, 1, lineEditInitial);
    connect(lineEditInitial, SIGNAL(editingFinished()), this, SLOT(onInitialValueChanged()));

    QString current = QString::number(currentValue);
    QLineEdit* lineEditCurrent = new QLineEdit();
    lineEditCurrent->setText(current);
    lineEditCurrent->setValidator(mValidator);
    lineEditCurrent->setReadOnly(true);
    mTableWidget->setCellWidget(row, 2, lineEditCurrent);
    //connect(lineEditCurrent, SIGNAL(editingFinished()), this, SLOT(onCurrentValueChanged()));
}
