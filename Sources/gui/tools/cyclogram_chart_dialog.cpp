#include "Headers/gui/tools/cyclogram_chart_dialog.h"
#include "Headers/gui/qcustomplot.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logger/Logger.h"

#include "Headers/logic/variable_controller.h"

#include <QtWidgets>
#include <QDateTime>

namespace
{
    static const qreal WIDTH = 600;
    static const qreal HEIGHT = 400;
    static const qreal X_AXIS_ADD = 30;
}

CyclogramChartDialog::CyclogramChartDialog(QWidget * parent):
    QDialog(parent)
{
    QHBoxLayout* hLayout = new QHBoxLayout(this);

    mVariablesTable = new QTableWidget(this);
    QStringList list;
    list.append(tr("Name"));
    list.append(tr("Value"));
    mVariablesTable->setColumnCount(list.size());
    mVariablesTable->setHorizontalHeaderLabels(list);
    mVariablesTable->horizontalHeader()->setStretchLastSection(true);
    mVariablesTable->setMinimumWidth(200);
    mVariablesTable->setMaximumWidth(300);
    mVariablesTable->setSelectionMode(QAbstractItemView::NoSelection);

    hLayout->addWidget(mVariablesTable);

    QCustomPlot* plot = new QCustomPlot(this);
    plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
    hLayout->addWidget(plot);
    mPlot = plot;

    setLayout(hLayout);

    setWindowTitle(tr("Main cyclogram")); // TODO write cyclogram path / subprogram name
}

CyclogramChartDialog::~CyclogramChartDialog()
{

}

void CyclogramChartDialog::setCyclogram(QSharedPointer<Cyclogram> cyclogram, const QStringList& variables)
{
    if (cyclogram.isNull())
    {
        return;
    }

    if (mCyclogram.lock())
    {
        disconnect(mCyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCyclogram.lock()->variableController(), SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
        disconnect(mCyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));
    }

    mCyclogram = cyclogram;

    mVariablesTable->clearContents();
    mVariablesTable->setRowCount(0);
    mPlot->clearGraphs();
    mPlot->legend->setVisible(false);
    mPlot->replot();

    auto variablesToDisplay = variables.toSet();

    const QMap<QString, VariableController::VariableData>& data = cyclogram->variableController()->variablesData();

    int row = 0;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (!variablesToDisplay.contains(it.key()))
        {
            continue;
        }

        addRow(row, it.key(), it.value().currentValue);
        ++row;
    }

    //connect(cyclogram->variableController(), SIGNAL(dataSnapshotAdded(const VariableController::DataSnapshot&)), this, SLOT(updateGraphs(const VariableController::DataSnapshot&)));
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    if (cyclogram->state() == Cyclogram::RUNNING) //TODO "hot" graphs adding
    {
        onCyclogramStateChanged(Cyclogram::RUNNING);
    }
}

void CyclogramChartDialog::onVariableValueChanged(const QString& name, qreal value)
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

    if (!graph)
    {
        LOG_DEBUG(QString("Graph for variable '%1' not found").arg(name));
        return;
    }

    // update variable value in table
    for(int row = 0; row < mVariablesTable->rowCount(); row++)
    {
        QLineEdit* tmp = qobject_cast<QLineEdit*>(mVariablesTable->cellWidget(row, 0));
        if (!tmp)
        {
            continue;
        }

        if (tmp->text() == name)
        {
            tmp = qobject_cast<QLineEdit*>(mVariablesTable->cellWidget(row, 1));
            tmp->setText(QString::number(value));
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

    mPlot->yAxis->setRange(mMinY, mMaxY); //TODO all negative values?

    mPlot->replot();
}

/*void MonitorAuto::updateGraphs(const VariableController::DataSnapshot& data)
{
    qreal minValue;
    qreal maxValue;

    qreal time = qreal(data.timestamp - mStartTime) / 1000;

    int i = 0;
    for (auto it = data.variables.begin(); it != data.variables.end(); ++it)
    {
        if (i == 0)
        {
            minValue = it.value();
            maxValue = it.value();
        }
        else
        {
            if (it.value() < minValue)
            {
                minValue = it.value();
            }
            else if (it.value() > maxValue)
            {
                maxValue = it.value();
            }
        }

        QCPGraph* graph = mPlot->graph(i);
        graph->addData(time, it.value());
        ++i;
    }

    // if graphs reached right plot point, expand it
    if (time > mMaxX)
    {
        mMaxX += X_AXIS_ADD;
        mPlot->xAxis->setRangeUpper(mMaxX);
    }

    // update Y axis ranges to new values
    if (minValue < mMinY)
    {
        mMinY = minValue;
    }
    else if (maxValue > mMaxY)
    {
        mMaxY = maxValue;
    }

    mPlot->yAxis->setRange(mMinY, mMaxY); //TODO all negative values?

    mPlot->replot();
}*/

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
        mPlot->clearGraphs(); //TODO не очищать графики подпрограм, а сдлеать метод для очистки графиков/рестарта

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

        const QMap<QString, VariableController::VariableData>& data = vc->variablesData();

        int color = Qt::red;

        for(int row = 0; row < mVariablesTable->rowCount(); row++)
        {
            QLineEdit* varNameEdit = qobject_cast<QLineEdit*>(mVariablesTable->cellWidget(row, 0));
            QCPGraph* graph = mPlot->addGraph();
            graph->setName(tr("%1").arg(varNameEdit->text()));
            graph->setPen(QPen(Qt::GlobalColor(color)));
            graph->addData(mMinX, vc->currentValue(varNameEdit->text()));
            ++color;

            if (color > Qt::darkYellow)
            {
                color = Qt::red; //TODO temporary
            }
        }

        mPlot->legend->setVisible(true);
        mPlot->replot();

        connect(vc, SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
    }
    else if (state == Cyclogram::STOPPED)
    {
        //TODO some hacks with subprogram charts
    }
}

void CyclogramChartDialog::addRow(int row, const QString& name, qreal value)
{
    mVariablesTable->insertRow(row);

    QLineEdit* lineEditName = new QLineEdit(mVariablesTable);
    lineEditName->setFrame(false);
    lineEditName->setText(name);
    lineEditName->setReadOnly(true);
    mVariablesTable->setCellWidget(row, 0, lineEditName);

    QLineEdit* lineEditInitial = new QLineEdit(mVariablesTable);
    lineEditInitial->setFrame(false);
    lineEditInitial->setText(QString::number(value));
    lineEditInitial->setReadOnly(true);
    mVariablesTable->setCellWidget(row, 1, lineEditInitial);
}
