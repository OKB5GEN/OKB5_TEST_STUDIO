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
    QDialog(parent),
    mCyclogram(Q_NULLPTR)
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

            /*
    QVBoxLayout * vLayout = new QVBoxLayout(this);

    {// value to monitor select
        QHBoxLayout *hLayout = new QHBoxLayout(this);
        QComboBox* comboBox = new QComboBox(this);
        comboBox->addItem("Current voltage");
        comboBox->addItem("Temperature 1");
        comboBox->addItem("Temperature 2");
        comboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        hLayout->addWidget(comboBox);

        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Current value, Units");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(activeCaption);

        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit->setAlignment(Qt::AlignRight);
        lineEdit->setText(QString::number(0));
        lineEdit->setReadOnly(true);
        hLayout->addWidget(lineEdit);

        vLayout->addLayout(hLayout);
    }

    { // current value
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);

        // play button
        QToolButton* playBtn = new QToolButton(this);
        playBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        playBtn->setIcon(QIcon(":/images/media-play-32.png"));
        playBtn->setIconSize(QSize(64, 64));
        connect(playBtn, SIGNAL(clicked()), this, SLOT(onPlayClicked()));
        hLayout->addWidget(playBtn);

        // pause button
        QToolButton* pauseBtn = new QToolButton(this);
        pauseBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        pauseBtn->setIcon(QIcon(":/images/media-pause-32.png"));
        pauseBtn->setIconSize(QSize(64, 64));
        connect(pauseBtn, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
        hLayout->addWidget(pauseBtn);

        // stop button
        QToolButton* stopBtn = new QToolButton(this);
        stopBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        stopBtn->setIcon(QIcon(":/images/media-stop-32.png"));
        stopBtn->setIconSize(QSize(64, 64));
        connect(stopBtn, SIGNAL(clicked()), this, SLOT(onStopClicked()));
        hLayout->addWidget(stopBtn);

        hLayout->addStretch();

        // value restrictions group box
        QGroupBox *groupBox = new QGroupBox(tr("Restrictions"), this);
        groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

        QGridLayout* boxLayout = new QGridLayout(this);

        QCheckBox* activeBox1 = new QCheckBox(this);
        activeBox1->setChecked(false);
        activeBox1->setText("Min");
        activeBox1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        boxLayout->addWidget(activeBox1, 0, 0);

        QLineEdit* lineEdit1 = new QLineEdit(this);
        lineEdit1->setValidator(new QIntValidator(this));
        lineEdit1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit1->setText(QString::number(0));
        lineEdit1->setEnabled(false);
        lineEdit1->setAlignment(Qt::AlignRight);
        boxLayout->addWidget(lineEdit1, 0, 1);

        QCheckBox* activeBox2 = new QCheckBox(this);
        activeBox2->setChecked(false);
        activeBox2->setText("Max");
        activeBox2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        boxLayout->addWidget(activeBox2, 1, 0);

        QLineEdit* lineEdit2 = new QLineEdit(this);
        lineEdit2->setValidator(new QIntValidator(this));
        lineEdit2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit2->setText(QString::number(0));
        lineEdit2->setEnabled(false);
        lineEdit2->setAlignment(Qt::AlignRight);
        boxLayout->addWidget(lineEdit2, 1, 1);

        groupBox->setLayout(boxLayout);

        hLayout->addWidget(groupBox);
        vLayout->addLayout(hLayout);
    }

    { // refresh period
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);

        QLabel* activeLabel = new QLabel(this);
        activeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeLabel->setText("Refresh period, sec");
        hLayout->addWidget(activeLabel);

        QLineEdit* timeEdit = new QLineEdit(this);
        timeEdit->setAlignment(Qt::AlignRight);
        timeEdit->setValidator(new QIntValidator(MIN_UPDATE_INTERVAL, MAX_UPDATE_INTERVAL, this));
        timeEdit->setText(QString::number(MIN_UPDATE_INTERVAL));
        timeEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(timeEdit);

        connect(timeEdit, SIGNAL(textChanged(QString)), this, SLOT(setUpdatePeriod(QString)));

        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(true);
        activeBox->setText("Show plot");
        activeBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        connect(activeBox, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
        hLayout->addWidget(activeBox);

        mPlotCheckBox = activeBox;

        hLayout->addStretch();
        vLayout->addLayout(hLayout);
    }

    { // plot

        QCustomPlot* plot = new QCustomPlot(this);
        plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
        vLayout->addWidget(plot, 10);
        mPlot = plot;
    }

    vLayout->addStretch();

    setLayout(vLayout);
            */

    setWindowTitle(tr("Monitor"));
}

MonitorAuto::~MonitorAuto()
{

}

void MonitorAuto::setCyclogram(Cyclogram * cyclogram)
{
    if (!cyclogram)
    {
        return;
    }

    if (mCyclogram)
    {
        disconnect(mCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
        disconnect(mCyclogram->variableController(), SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
    }

    mCyclogram = cyclogram;

    mCheckboxes.clear();
    mVariables->clear();
    mPlot->clearGraphs();
    mPlot->legend->setVisible(false);
    mPlot->replot();

    const QMap<QString, VariableController::VariableData>& data = mCyclogram->variableController()->variablesData();

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
    connect(mCyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));

    if (cyclogram->state() == Cyclogram::RUNNING) //TODO
    {
        onCyclogramStateChanged(Cyclogram::RUNNING);
    }
}

void  MonitorAuto::onVariableSelectionChanged(bool toggled)
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

    disconnect(mCyclogram->variableController(), SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));

    if (state == Cyclogram::RUNNING)
    {
        mPlot->clearGraphs();

        mStartTime = QDateTime::currentMSecsSinceEpoch();
        QFont font;
        font.setPointSize(10);
        font.setFamily("Verdana");

        mPlot->xAxis->setTickLabelFont(font);
        mPlot->xAxis->setLabelFont(font);
        mPlot->xAxis->setLabel(tr("Время, мс"));

        mPlot->yAxis->setTickLabelFont(font);
        mPlot->yAxis->setLabelFont(font);
        mPlot->yAxis->setLabel(tr("Значение"));

        mMinY = 0;
        mMaxY = 0;
        mMinX = 0;
        mMaxX = X_AXIS_ADD;

        int TODO;
        mPlot->xAxis->setRange(mMinX, mMaxX);//Для оси Ox
        mPlot->yAxis->setRange(mMinY, mMaxY);//Для оси Oy

        VariableController* vc = mCyclogram->variableController();

        const QMap<QString, VariableController::VariableData>& data = vc->variablesData();

        int i = 0;
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

        connect(mCyclogram->variableController(), SIGNAL(currentValueChanged(const QString&, qreal)), this, SLOT(onVariableValueChanged(const QString&, qreal)));
    }
}
