#include "Headers/gui/tools/monitor_auto.h"
#include "Headers/gui/qcustomplot.h"
#include "Headers/logic/cyclogram.h"

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
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(getCurrentValue()));

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
    setWindowTitle(tr("Monitor"));

    updateUI();
}

MonitorAuto::~MonitorAuto()
{

}

//void MonitorAuto::paintEvent(QPaintEvent *event)
//{

//}

//void MonitorAuto::mousePressEvent(QMouseEvent *event)
//{

//}

//void MonitorAuto::mouseReleaseEvent(QMouseEvent *event)
//{

//}

//void MonitorAuto::mouseMoveEvent(QMouseEvent *event)
//{

//}

void MonitorAuto::setCyclogram(Cyclogram * cyclogram)
{
    if (!cyclogram)
    {
        return;
    }

    mCyclogram = cyclogram;
    connect(cyclogram->variableController(), SIGNAL(dataSnapshotAdded(const VariableController::DataSnapshot&)), this, SLOT(updateGraphs(const VariableController::DataSnapshot&)));
    connect(cyclogram, SIGNAL(stateChanged(int)), this, SLOT(onCyclogramStateChanged(int)));
}

void MonitorAuto::updateGraphs(const VariableController::DataSnapshot& data)
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
}

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
            ++color;

            if (color > Qt::darkYellow)
            {
                color = Qt::red; //TODO temporary
            }
        }

        mPlot->legend->setVisible(true);
        mPlot->replot();
    }
}

void MonitorAuto::onPlayClicked()
{
    return;

    double a = 0; //Начало интервала, где рисуем график по оси Ox
    double b = 100; //Конец интервала, где рисуем график по оси Ox
    int N = 100 + 1; //Вычисляем количество точек, которые будем отрисовывать

    //Массивы координат точек
    QVector<double> x(N);
    QVector<double> y1(N);
    QVector<double> y2(N);
    int i = 0;

    for (double X = 0; X < b; X++)//Пробегаем по всем точкам
    {
        x[i] = X * 2;
        y1[i] = fRand(1.0, 10.0);
        y2[i] = fRand(1.0, 10.0);
        i++;
    }

    mPlot->clearGraphs();//Если нужно, то очищаем все графики

    mPlot->addGraph();
    mPlot->graph(0)->setData(x, y1);
    mPlot->graph(0)->setPen(QPen(Qt::blue));
    mPlot->graph(0)->setName("I(БУП НА),A ");

    mPlot->addGraph();
    mPlot->graph(1)->setPen(QPen(Qt::red));
    mPlot->graph(1)->setData(x, y2);
    mPlot->graph(1)->setName("I(ПНА),A ");

    mPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 10));
    mPlot->xAxis->setLabelFont(QFont(QFont().family(), 10));
    mPlot->xAxis->setLabel("Время, с");
    mPlot->xAxis->setRange(a, 150);//Для оси Ox

    mPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 10));
    mPlot->yAxis->setLabelFont(QFont(QFont().family(), 10));
    mPlot->yAxis->setLabel(" I изм, А");
    mPlot->yAxis->setRange(-0.5, 10.5);//Для оси Oy

    mPlot->legend->setVisible(true);
    mPlot->replot();
}

void MonitorAuto::onPauseClicked()
{
    //TODO
}

void MonitorAuto::onStopClicked()
{
    mPlot->clearGraphs();
    mPlot->replot();
    //TODO
}

void MonitorAuto::setUpdatePeriod(QString period)
{
    mUpdatePeriod = period.toInt();
    //TODO timer
}

void MonitorAuto::updateUI()
{
    mPlot->setVisible(mPlotCheckBox->isChecked());

    adjustSize();
    update();
}

void MonitorAuto::getCurrentValue()
{
    //TODO
}
