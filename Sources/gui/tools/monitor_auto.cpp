#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/gui/qcustomplot.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logger/Logger.h"

#include "Headers/logic/variable_controller.h"

#include <QtWidgets>
#include <QDateTime>

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;
    static const qreal X_AXIS_ADD = 30;

    static const qreal BTN_SIZE = 50;

    static const int MIN_UPDATE_INTERVAL = 1; // each second
    static const int MAX_UPDATE_INTERVAL = INT_MAX;

    double fRand(double fMin, double fMax)
    {
        double f = (double)rand() / RAND_MAX;
        return fMin + f * (fMax - fMin);
    }
}

MonitorAuto::MonitorAuto(QWidget * parent):
    QDialog(parent)
{
    QHBoxLayout* hLayout = new QHBoxLayout(this);

    mVariables = new QListWidget(this);
    mVariables->setMaximumWidth(100);

    hLayout->addWidget(mVariables);

    QCustomPlot* plot = new QCustomPlot(this);
    plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
    hLayout->addWidget(plot);
    mPlot = plot;

    setLayout(hLayout);

    setWindowTitle(tr("Main cyclogram")); // TODO write cyclogram path / subprogram name
}

MonitorAuto::~MonitorAuto()
{

}

void MonitorAuto::setCyclogram(QSharedPointer<Cyclogram> cyclogram)
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

    mCheckboxes.clear();
    mVariables->clear();
    mPlot->clearGraphs();
    mPlot->legend->setVisible(false);
    mPlot->replot();

    const QMap<QString, VariableController::VariableData>& data = cyclogram->variableController()->variablesData();

    for (auto it = data.begin(); it != data.end(); ++it)
    {
        QCheckBox* checkBox = new QCheckBox(this);
        checkBox->setText(it.key());
        mCheckboxes[it.key()] = checkBox;

        connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(onVariableSelectionChanged(bool)));

        QListWidgetItem* item = new QListWidgetItem();
        mVariables->addItem(item);
        mVariables->setItemWidget(item, checkBox);
    }

    //connect(cyclogram->variableController(), SIGNAL(dataSnapshotAdded(const VariableController::DataSnapshot&)), this, SLOT(updateGraphs(const VariableController::DataSnapshot&)));
    connect(cyclogram.data(), SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
    connect(cyclogram.data(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    if (cyclogram->state() == Cyclogram::RUNNING) //TODO "hot" graphs adding
    {
        onCyclogramStateChanged(Cyclogram::RUNNING);
    }
}

void MonitorAuto::onVariableSelectionChanged(bool toggled)
{
    QCheckBox* changedBox = qobject_cast<QCheckBox*>(QObject::sender());
    if (changedBox && !toggled)
    {
        QString name = changedBox->text();
        int count = mPlot->graphCount();
        QCPGraph* graph = Q_NULLPTR;

        for (int i = 0; i < count; ++i)
        {
            QCPGraph* tmp = mPlot->graph(i);
            if (tmp->name() == name)
            {
                tmp->setVisible(false);
                break;
            }
        }
    }
}

void MonitorAuto::onVariableValueChanged(const QString& name, qreal value)
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
        LOG_DEBUG(QString("Graph for variable '%1' not found").arg(name)); //TODO variable not included in display list?
        return;
    }

    qreal time = qreal(QDateTime::currentMSecsSinceEpoch() - mStartTime) / 1000;
    graph->addData(time, value);

    QCheckBox* checkBox = mCheckboxes.value(name);
    graph->setVisible(checkBox->isChecked());

    if (!checkBox->isChecked())
    {
        mPlot->replot();
        return; // variable not selected
    }

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

void MonitorAuto::onCyclogramStateChanged(int state)
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
        mPlot->xAxis->setLabel(tr("Время, с"));

        mPlot->yAxis->setTickLabelFont(font);
        mPlot->yAxis->setLabelFont(font);
        mPlot->yAxis->setLabel(tr("Значение"));

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
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            QCPGraph* graph = mPlot->addGraph();
            graph->setName(tr("%1").arg(it.key()));
            graph->setPen(QPen(Qt::GlobalColor(color)));
            graph->addData(mMinX, it.value().currentValue);
            ++color;

            if (color > Qt::darkYellow)
            {
                color = Qt::red; //TODO temporary
            }

            bool isChecked = mCheckboxes.value(it.key())->isChecked();
            graph->setVisible(isChecked);
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
