#include "Headers/variables_window.h"
#include "Headers/variable_controller.h"

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
    mTableWidget->setFixedWidth(mTableWidget->sizeHint().width());

    layout->addWidget(mTableWidget, 0, 0, 5, 5);

    // play button
    QToolButton* addBtn = new QToolButton(this);
    addBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    addBtn->setIcon(QIcon(":/images/edit_add.png"));
    addBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    connect(addBtn, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    layout->addWidget(addBtn, 0, 5, 1, 1);

    // pause button
    QToolButton* removeBtn = new QToolButton(this);
    removeBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    removeBtn->setIcon(QIcon(":/images/edit_remove.png"));
    removeBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    connect(removeBtn, SIGNAL(clicked()), this, SLOT(onRemoveClicked()));
    layout->addWidget(removeBtn, 1, 5, 1, 1);

    setLayout(layout);
    setWindowTitle(tr("Variables"));
}

VariablesWindow::~VariablesWindow()
{

}

void VariablesWindow::setVariableController(VariableController * controller)
{
    mController = controller;

    mTableWidget->clearContents();
    mTableWidget->setRowCount(0);

    connect(mController, SIGNAL(valueChanged(const QString&,qreal,int)), this, SLOT(onValueChanged(const QString&,qreal,int)));

    foreach (QString key, mController->variables().keys())
    {
        int index = mTableWidget->rowCount();
        qreal initial = mController->variable(key, -1, VariableController::Initial);
        qreal current = mController->variable(key, -1, VariableController::Current);
        addRow(index, key, initial, current);
    }
}

void VariablesWindow::onAddClicked()
{
    QString prefix = "V";
    QString name = prefix;
    int index = 0;

    while (mController->isVariableExist(name))
    {
        ++index;
        name = prefix + QString::number(index);
    }

    addRow(mTableWidget->rowCount(), name, 0, 0);
    mController->addVariable(name, 0);
}

void VariablesWindow::onRemoveClicked()
{
    int TODO;
}

void VariablesWindow::onNameChanged()
{
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

    if (mController->isVariableExist(newName))
    {
        // name is not unique, restore old name
        QMessageBox::warning(this, tr("Error"), tr("Variable '%1' already exist").arg(newName));
        lineEdit->setText(oldName);
        return;
    }

    lineEdit->setProperty(PREV_NAME_PROPERTY, QVariant(newName));
    mController->renameVariable(newName, oldName);
}

void VariablesWindow::onInitialValueChanged()
{
    QLineEdit* valueLineEdit = qobject_cast<QLineEdit*>(QObject::sender());
    if (!valueLineEdit)
    {
        qDebug("WTF 2?");
        return;
    }

    qreal value = valueLineEdit->text().toDouble();

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

    mController->setVariable(name, value, VariableController::Initial);
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
    lineEditInitial->setValidator(new QDoubleValidator());
    mTableWidget->setCellWidget(row, 1, lineEditInitial);
    connect(lineEditInitial, SIGNAL(editingFinished()), this, SLOT(onInitialValueChanged()));

    QString current = QString::number(currentValue);
    QLineEdit* lineEditCurrent = new QLineEdit();
    lineEditCurrent->setText(current);
    lineEditCurrent->setValidator(new QDoubleValidator());
    lineEditCurrent->setReadOnly(true);
    mTableWidget->setCellWidget(row, 2, lineEditCurrent);
    //connect(lineEditCurrent, SIGNAL(editingFinished()), this, SLOT(onCurrentValueChanged()));
}

void VariablesWindow::removeRow(int row)
{
    int TODO;
}
