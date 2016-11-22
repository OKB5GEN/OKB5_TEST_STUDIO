#include "Headers/monitordialog.h"
#include "qcustomplot.h"

#include <QtWidgets>

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;

    static const qreal BTN_SIZE = 50;

    static const int MIN_UPDATE_INTERVAL = 1; // each second
    static const int MAX_UPDATE_INTERVAL = INT_MAX;
}

MonitorDialog::MonitorDialog(QWidget * parent):
    QDialog(parent),
    mIsAutoMode(true)
{
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(getCurrentValue()));

    if (parent != Q_NULLPTR)
    {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        setGeometry(parentPos.x() + parent->width() / 2 - WIDTH / 2,
                    parentPos.y() + parent->height() / 2 - HEIGHT / 2,
                    WIDTH, HEIGHT);
    }


    QVBoxLayout * layout = new QVBoxLayout(this);

    {// value to monitor select
        QHBoxLayout *hLayout = new QHBoxLayout(this);
        QComboBox* comboBox = new QComboBox(this);
        comboBox->addItem("Current voltage");
        comboBox->addItem("Temperature 1");
        comboBox->addItem("Temperature 2");
        comboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(comboBox);

        QGroupBox *groupBox = new QGroupBox(tr("Mode"), this);
        groupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QRadioButton *radio1 = new QRadioButton(tr("Automatic"), this);
        QRadioButton *radio2 = new QRadioButton(tr("Manual"), this);
        radio1->setChecked(mIsAutoMode);
        radio1->setChecked(!mIsAutoMode);
        connect(radio1, SIGNAL(clicked()), this, SLOT(setAutoMode()));
        connect(radio2, SIGNAL(clicked()), this, SLOT(setManualMode()));

        QVBoxLayout *hbox = new QVBoxLayout(this);
        hbox->addWidget(radio1);
        hbox->addWidget(radio2);
        groupBox->setLayout(hbox);

        hLayout->addWidget(groupBox);
        layout->addLayout(hLayout);
    }

    { // current value
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);

        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Current value");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(activeCaption);

        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit->setAlignment(Qt::AlignRight);
        lineEdit->setText(QString::number(0));
        lineEdit->setReadOnly(true);
        hLayout->addWidget(lineEdit);

        QToolButton* refreshBtn = new QToolButton(this);
        refreshBtn->setText(tr("Refresh"));
        connect(refreshBtn, SIGNAL(clicked()), this, SLOT(onPlayClicked()));
        hLayout->addWidget(refreshBtn);

        mManualModeWidgets.push_back(refreshBtn);

        QGroupBox *groupBox = new QGroupBox(tr("Restrictions"), this);
        mAutoModeWidgets.push_back(groupBox);

        /////////
        QHBoxLayout* hLayout1 = new QHBoxLayout(this);
        hLayout1->setAlignment(Qt::AlignLeft);

        QCheckBox* activeBox1 = new QCheckBox(this);
        activeBox1->setChecked(false);
        activeBox1->setText("Min");
        activeBox1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout1->addWidget(activeBox1);

        QLineEdit* lineEdit1 = new QLineEdit(this);
        lineEdit1->setValidator(new QIntValidator(this));
        lineEdit1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit1->setText(QString::number(0));
        lineEdit1->setEnabled(false);
        lineEdit1->setAlignment(Qt::AlignRight);
        hLayout1->addWidget(lineEdit1);

        mAutoModeWidgets.push_back(activeBox1);
        mAutoModeWidgets.push_back(lineEdit1);
        ///////

        QHBoxLayout* hLayout2 = new QHBoxLayout(this);
        hLayout2->setAlignment(Qt::AlignLeft);

        QCheckBox* activeBox2 = new QCheckBox(this);
        activeBox2->setChecked(false);
        activeBox2->setText("Max");
        activeBox2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout2->addWidget(activeBox2);

        QLineEdit* lineEdit2 = new QLineEdit(this);
        lineEdit2->setValidator(new QIntValidator(this));
        lineEdit2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEdit2->setText(QString::number(0));
        lineEdit2->setEnabled(false);
        lineEdit2->setAlignment(Qt::AlignRight);
        hLayout2->addWidget(lineEdit2);

        mAutoModeWidgets.push_back(activeBox2);
        mAutoModeWidgets.push_back(lineEdit2);
        /////////

        QVBoxLayout *vbox = new QVBoxLayout(this);
        vbox->addLayout(hLayout1);
        vbox->addLayout(hLayout2);
        groupBox->setLayout(vbox);

        hLayout->addWidget(groupBox);

        hLayout->addStretch();
        layout->addLayout(hLayout);
    }

    { // buttons
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);

        QToolButton* playBtn = new QToolButton(this);
        playBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        playBtn->setIcon(QIcon(":/images/media-play-32.png"));
        playBtn->setIconSize(QSize(64, 64));
        connect(playBtn, SIGNAL(clicked()), this, SLOT(onPlayClicked()));
        hLayout->addWidget(playBtn);

        QToolButton* pauseBtn = new QToolButton(this);
        pauseBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        pauseBtn->setIcon(QIcon(":/images/media-pause-32.png"));
        pauseBtn->setIconSize(QSize(64, 64));
        connect(pauseBtn, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
        hLayout->addWidget(pauseBtn);

        QToolButton* stopBtn = new QToolButton(this);
        stopBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
        stopBtn->setIcon(QIcon(":/images/media-stop-32.png"));
        stopBtn->setIconSize(QSize(64, 64));
        connect(stopBtn, SIGNAL(clicked()), this, SLOT(onStopClicked()));
        hLayout->addWidget(stopBtn);

        hLayout->addStretch();
        layout->addLayout(hLayout);

        mAutoModeWidgets.push_back(playBtn);
        mAutoModeWidgets.push_back(pauseBtn);
        mAutoModeWidgets.push_back(stopBtn);
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

        mAutoModeWidgets.push_back(activeLabel);
        mAutoModeWidgets.push_back(timeEdit);
        mAutoModeWidgets.push_back(activeBox);

        hLayout->addStretch();
        layout->addLayout(hLayout);
    }

    { // plot

        QCustomPlot* plot = new QCustomPlot(this);
        plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
        layout->addWidget(plot);
        mPlot = plot;
        mAutoModeWidgets.push_back(plot);
    }

    layout->addStretch();

    setLayout(layout);
    setWindowTitle("Monitor");

    updateUI();
}

MonitorDialog::~MonitorDialog()
{

}

void MonitorDialog::paintEvent(QPaintEvent *event)
{

}

void MonitorDialog::mousePressEvent(QMouseEvent *event)
{

}

void MonitorDialog::mouseReleaseEvent(QMouseEvent *event)
{

}

void MonitorDialog::mouseMoveEvent(QMouseEvent *event)
{

}

void MonitorDialog::onPlayClicked()
{
    //TODO
}

void MonitorDialog::onPauseClicked()
{
    //TODO
}

void MonitorDialog::onStopClicked()
{
    //TODO
}

void MonitorDialog::setUpdatePeriod(QString period)
{
    mUpdatePeriod = period.toInt();
    if (!mIsAutoMode)
    {
        return;
    }

    //TODO timer
}

void MonitorDialog::setAutoMode()
{
    mIsAutoMode = true;
    updateUI();
}

void MonitorDialog::setManualMode()
{
    mIsAutoMode = false;
    updateUI();

    mTimer->stop();
}

void MonitorDialog::updateUI()
{
    for (int i = 0, sz = mManualModeWidgets.size(); i < sz;++i)
    {
        mManualModeWidgets[i]->setVisible(!mIsAutoMode);
    }

    for (int i = 0, sz = mAutoModeWidgets.size(); i < sz; ++i)
    {
        mAutoModeWidgets[i]->setVisible(mIsAutoMode);
    }

    mPlot->setVisible(mPlotCheckBox->isChecked() && mIsAutoMode);

    adjustSize();
    update();
}

void MonitorDialog::getCurrentValue()
{
    //TODO
}
