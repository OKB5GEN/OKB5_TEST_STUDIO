#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/qcustomplot.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"
#include "Headers/gui/cyclogram/variables_select_window.h"

#include <QtWidgets>
#include <QDateTime>

namespace
{
    static const qreal WIDTH = 600;
    static const qreal HEIGHT = 400;
    static const qreal X_AXIS_ADD = 30;
    static const int BTN_SIZE = 32;
    static const qreal RANGE_FACTOR = 1.1;
}

CyclogramChartDialog::CyclogramChartDialog(QWidget * parent):
    RestorableDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* variablesLayout = new QVBoxLayout();
    mainLayout->addLayout(variablesLayout, 1);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    // add button
    mAddBtn = new QToolButton(this);
    mAddBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mAddBtn->setIcon(QIcon(":/resources/images/edit_add.png"));
    mAddBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mAddBtn->setToolTip(tr("Add existing variable"));
    connect(mAddBtn, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    buttonsLayout->addWidget(mAddBtn);

    // remove button
    mRemoveBtn = new QToolButton(this);
    mRemoveBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setIcon(QIcon(":/resources/images/edit_remove.png"));
    mRemoveBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mRemoveBtn->setToolTip(tr("Remove selected variables"));
    mRemoveBtn->setEnabled(false);
    connect(mRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemoveClicked()));
    buttonsLayout->addWidget(mRemoveBtn);

    // move up button
    mMoveUpBtn = new QToolButton(this);
    mMoveUpBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveUpBtn->setIcon(QIcon(":/resources/images/arrow_up.png"));
    mMoveUpBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveUpBtn->setToolTip(tr("Move selected variable up"));
    mMoveUpBtn->setEnabled(false);
    connect(mMoveUpBtn, SIGNAL(clicked()), this, SLOT(onMoveUpClicked()));
    buttonsLayout->addWidget(mMoveUpBtn);

    // move down button
    mMoveDownBtn = new QToolButton(this);
    mMoveDownBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveDownBtn->setIcon(QIcon(":/resources/images/arrow_down.png"));
    mMoveDownBtn->setIconSize(QSize(BTN_SIZE, BTN_SIZE));
    mMoveDownBtn->setToolTip(tr("Move selected variable down"));
    mMoveDownBtn->setEnabled(false);
    connect(mMoveDownBtn, SIGNAL(clicked()), this, SLOT(onMoveDownClicked()));
    buttonsLayout->addWidget(mMoveDownBtn);

    buttonsLayout->addStretch();

    variablesLayout->addLayout(buttonsLayout);

    mVariablesTable = new QTableWidget(this);
    QStringList list;
    list.append(tr("Name"));
    list.append(tr("Value"));
    mVariablesTable->setColumnCount(list.size());
    mVariablesTable->setHorizontalHeaderLabels(list);
    mVariablesTable->horizontalHeader()->setStretchLastSection(true);
    mVariablesTable->setMinimumWidth(300);

    connect(mVariablesTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onCellDoubleClicked(int, int)));
    connect(mVariablesTable, SIGNAL(itemSelectionChanged()), this, SLOT(onTableSelectionChanged()));
    mVariablesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    variablesLayout->addWidget(mVariablesTable);

    QCheckBox* showChartBox = new QCheckBox(tr("Show/hide charts"), this);
    connect(showChartBox, SIGNAL(stateChanged(int)), this, SLOT(onShowChartBoxStateChanged(int)));
    variablesLayout->addWidget(showChartBox);

    mPlot = new QCustomPlot(this);
    mPlot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
    mainLayout->addWidget(mPlot, 10);

    setLayout(mainLayout);

    setWindowTitle(tr("Main cyclogram"));

    showChartBox->setCheckState(Qt::Checked); // must be after QCustomPlot creation
}

CyclogramChartDialog::~CyclogramChartDialog()
{

}

void CyclogramChartDialog::setCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    if (cyclogram.isNull())
    {
        return;
    }

    if (mCyclogram.lock())
    {
        VariableController* vc =  mCyclogram.lock()->variableController();
        disconnect(mCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(vc, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
        disconnect(vc, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onVariableNameChanged(const QString&, const QString&)));
        disconnect(vc, SIGNAL(variableRemoved(const QString&)), this, SLOT(onVariableRemoved(const QString&)));
        disconnect(vc, SIGNAL(variableAdded(const QString&, qreal)), this, SLOT(onVariableAdded(const QString&, qreal)));
        disconnect(mCyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));
    }

    mCyclogram = cyclogram;

    mVariablesTable->clearContents();
    mVariablesTable->setRowCount(0);
    mPlot->clearGraphs();
    mPlot->legend->setVisible(false);
    mPlot->replot();

    VariableController* vc =  cyclogram->variableController();
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));
    connect(vc, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onVariableNameChanged(const QString&, const QString&)));
    connect(vc, SIGNAL(variableRemoved(const QString&)), this, SLOT(onVariableRemoved(const QString&)));
    connect(vc, SIGNAL(variableAdded(const QString&, qreal)), this, SLOT(onVariableAdded(const QString&, qreal)));

    if (cyclogram->state() == Cyclogram::RUNNING) //TODO "hot" graphs adding
    {
        onCyclogramStateChanged(Cyclogram::RUNNING);
    }

    readSettings();
}

QCPGraph* CyclogramChartDialog::variableGraph(const QString& name) const
{
    int count = mPlot->graphCount();
    QCPGraph* graph = Q_NULLPTR;

    for (int i = 0; i < count; ++i)
    {
        QCPGraph* tmp = mPlot->graph(i);
        if (tmp->name() == name)
        {
            graph = tmp;
            break;
        }
    }

    return graph;
}

void CyclogramChartDialog::onVariableValueChanged(const QString& name, qreal value)
{
    QCPGraph* graph = variableGraph(name);

    if (!graph)
    {
        //LOG_ERROR(QString("Graph for variable '%1' not found").arg(name));
        return; // normal situation, "no plot graph" option can be selected for variable
    }

    // update variable value in table
    for(int row = 0; row < mVariablesTable->rowCount(); row++)
    {
        QString variableName = mVariablesTable->item(row, 0)->text();
        if (variableName == name)
        {
            mVariablesTable->item(row, 1)->setText(QString::number(value));
        }
    }

    qreal time = qreal(QDateTime::currentMSecsSinceEpoch() - mStartTime) / 1000;
    graph->addData(time, value);

    // if graphs reached right plot point, expand it
    if (time > mMaxX)
    {
        mMaxX += X_AXIS_ADD;
        mPlot->xAxis->setRangeUpper(mMaxX);
    }

    // update Y axis ranges to new values
    if (value < mMinY)
    {
        mMinY = value;
    }
    else if (value > mMaxY)
    {
        mMaxY = value;
    }

    mPlot->yAxis->setRange(mMinY * RANGE_FACTOR, mMaxY * RANGE_FACTOR);

    mPlot->replot();
}

void CyclogramChartDialog::onCyclogramStateChanged(int state)
{
    /*red,
    green,
    blue,
    cyan,
    magenta,
    yellow,
    darkRed,
    darkGreen,
    darkBlue,
    darkCyan,
    darkMagenta,
    darkYellow,*/

    auto cyclogram = mCyclogram.lock();
    VariableController* vc = cyclogram->variableController();

    disconnect(vc, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));

    if (state == Cyclogram::RUNNING)
    {
        mPlot->clearGraphs(); //TODO не очищать графики подпрограм, а сделать метод для очистки графиков/рестарта

        mStartTime = QDateTime::currentMSecsSinceEpoch();
        QFont font;
        font.setPointSize(10);
        font.setFamily("Verdana");

        mPlot->xAxis->setTickLabelFont(font);
        mPlot->xAxis->setLabelFont(font);
        mPlot->xAxis->setLabel(tr("Time, s"));

        mPlot->yAxis->setTickLabelFont(font);
        mPlot->yAxis->setLabelFont(font);
        mPlot->yAxis->setLabel(tr("Value"));

        mMinY = 0;
        mMaxY = 0;
        mMinX = 0;
        mMaxX = X_AXIS_ADD;

        int TODO; // добавить какое-то управление минимумами-максимумами чтобы график прыгал по возможности пореже
        // либо со скроллингом делать график

        mPlot->xAxis->setRange(mMinX, mMaxX);//Для оси Ox
        mPlot->yAxis->setRange(mMinY, mMaxY);//Для оси Oy

        //const QMap<QString, VariableController::VariableData>& data = vc->variablesData();

        int color = Qt::red;

        for(int row = 0; row < mVariablesTable->rowCount(); row++)
        {
            QTableWidgetItem* item = mVariablesTable->item(row, 0);
            if (!item)
            {
                LOG_ERROR(QString("Invalid item detected! Row: %1, Column: %2").arg(row).arg(0));
                continue;
            }

            QString variableName = item->text();
            QCPGraph* graph = mPlot->addGraph();
            graph->setName(variableName);
            graph->setPen(QPen(Qt::GlobalColor(color)));

            qreal currentValue = vc->currentValue(variableName);
            graph->addData(mMinX, currentValue);
            ++color;

            if (color > Qt::darkYellow)
            {
                color = Qt::red; //TODO temporary
            }

            onVariableValueChanged(variableName, currentValue);
        }

        mPlot->legend->setVisible(true);
        mPlot->replot();

        connect(vc, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
    }
    else if (state == Cyclogram::IDLE || state == Cyclogram::PENDING_FOR_START)
    {
        //TODO some hacks with subprogram charts
    }
}

void CyclogramChartDialog::addRow(int row, const QString& name, qreal value)
{
    mVariablesTable->insertRow(row);

    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    mVariablesTable->setItem(row, 0, nameItem);

    QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(value));
    valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    mVariablesTable->setItem(row, 1, valueItem);
}

void CyclogramChartDialog::onAddClicked()
{
    auto cyclogram = mCyclogram.lock();
    VariablesSelectWindow varSelectWindow(this);

    QStringList varList;

    for(int row = 0; row < mVariablesTable->rowCount(); row++)
    {
        QString variableName = mVariablesTable->item(row, 0)->text();
        varList.append(variableName);
    }

    varSelectWindow.setCyclogram(cyclogram, varList);
    varSelectWindow.exec();

    int row = mVariablesTable->rowCount();
    VariableController* vc = cyclogram->variableController();

    foreach (QString var, varSelectWindow.selectedVariables())
    {
        addRow(row, var, vc->currentValue(var));
        ++row;
    }

    //mVariablesTable->sortByColumn(0);
    mAddBtn->setEnabled(mVariablesTable->rowCount() < vc->variablesData().size());
}

void CyclogramChartDialog::keyPressEvent(QKeyEvent *event)
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

void CyclogramChartDialog::onRemoveClicked()
{
    auto cyclogram = mCyclogram.lock();
    VariableController* vc = cyclogram->variableController();

    QList<int> selectedRows;
    QList<QString> selectedVariables;

    foreach (auto index, mVariablesTable->selectionModel()->selectedRows())
    {
        selectedRows.append(index.row());
        QTableWidgetItem* item = mVariablesTable->item(index.row(), 0);
        if (!item)
        {
            LOG_ERROR(QString("Invalid item detected! Row: %1, Column: %2").arg(index.row()).arg(0));
            continue;
        }

        selectedVariables.append(item->text());
    }

    int rowsDeleted = 0;
    qSort(selectedRows);
    foreach (int row, selectedRows)
    {
        mVariablesTable->removeRow(row - rowsDeleted);
        ++rowsDeleted;
    }

    foreach (QString name, selectedVariables)
    {
        removeVariableGraph(name);
    }

    mAddBtn->setEnabled(mVariablesTable->rowCount() < vc->variablesData().size());
}

void CyclogramChartDialog::onMoveUpClicked()
{
    int rowFrom = mVariablesTable->selectionModel()->selectedRows().at(0).row();

    QString variable = mVariablesTable->item(rowFrom, 0)->text();
    qreal value = mVariablesTable->item(rowFrom, 1)->text().toDouble();

    mVariablesTable->removeRow(rowFrom);

    addRow(rowFrom - 1, variable, value);
    mVariablesTable->selectRow(rowFrom - 1);
}

void CyclogramChartDialog::onMoveDownClicked()
{
    int rowFrom = mVariablesTable->selectionModel()->selectedRows().at(0).row();

    QString variable = mVariablesTable->item(rowFrom, 0)->text();
    qreal value = mVariablesTable->item(rowFrom, 1)->text().toDouble();

    mVariablesTable->removeRow(rowFrom);

    addRow(rowFrom + 1, variable, value);
    mVariablesTable->selectRow(rowFrom + 1);
}

void CyclogramChartDialog::onCellDoubleClicked(int row, int column)
{
    mVariablesTable->clearSelection();
    QTableWidgetItem* item = mVariablesTable->item(row, column);
    if (item)
    {
        item->setSelected(true);
    }
}

void CyclogramChartDialog::onTableSelectionChanged()
{
    int rowsSelected = mVariablesTable->selectionModel()->selectedRows().size();

    mRemoveBtn->setEnabled(rowsSelected > 0);
    if (rowsSelected == 1)
    {
        int row = mVariablesTable->selectionModel()->selectedRows().at(0).row();
        mMoveUpBtn->setEnabled(row > 0);
        mMoveDownBtn->setEnabled(row < (mVariablesTable->rowCount() - 1));
    }
    else
    {
        mMoveUpBtn->setEnabled(false);
        mMoveDownBtn->setEnabled(false);
    }
}

void CyclogramChartDialog::onVariableNameChanged(const QString& newName, const QString& oldName)
{
    for(int row = 0; row < mVariablesTable->rowCount(); row++)
    {
        QTableWidgetItem* item = mVariablesTable->item(row, 0);
        if (!item)
        {
            LOG_ERROR(QString("Invalid item detected! Row: %1, Column: %2").arg(row).arg(0));
            continue;
        }

        if (item->text() == oldName)
        {
            item->setText(newName);
            QCPGraph* graph = variableGraph(oldName);
            if (graph)
            {
                graph->setName(newName);
            }

            return;
        }
    }
}

void CyclogramChartDialog::onVariableRemoved(const QString& name)
{
    for(int row = 0; row < mVariablesTable->rowCount(); row++)
    {
        QTableWidgetItem* item = mVariablesTable->item(row, 0);
        if (!item)
        {
            LOG_ERROR(QString("Invalid item detected! Row: %1, Column: %2").arg(row).arg(0));
            continue;
        }

        if (item->text() == name)
        {
            mVariablesTable->removeRow(row);
            removeVariableGraph(name);
            return;
        }
    }
}

void CyclogramChartDialog::onVariableAdded(const QString& name, qreal value)
{
    mAddBtn->setEnabled(true);
}

void CyclogramChartDialog::removeVariableGraph(const QString& name)
{
    QCPGraph* graph = variableGraph(name);
    if (!graph)
    {
        return;
    }

    mPlot->removeGraph(graph);
}

void CyclogramChartDialog::onShowChartBoxStateChanged(int state)
{
    QSize curSize = this->size();
    QSize newSize = curSize;
    QSize tableSize = mVariablesTable->size();

    mPlot->setVisible(state == Qt::Checked);

//    QRect childRect = this->childrenRect();
//    QSize newSize;
//    newSize.setWidth(childRect.right() - childRect.left());
//    newSize.setHeight(childRect.bottom() - childRect.top());

    if (state != Qt::Checked)
    {
        newSize.setWidth(tableSize.width() + 50);
        resize(newSize);
    }

    adjustSize();
//    if (state == Qt::Checked)
//    {

//    }
//    else
//    {
//        curSize.setWidth(curSize.width() - plotSize.width());
//        resize(curSize);
//    }
}
