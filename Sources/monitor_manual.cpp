#include "Headers/monitor_manual.h"
#include "qcustomplot.h"

#include <QtWidgets>

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;

    static const qreal BTN_SIZE = 50;
}

MonitorManual::MonitorManual(QWidget * parent):
    QDialog(parent)
{
    if (parent != Q_NULLPTR)
    {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        setGeometry(parentPos.x() + parent->width() / 2 - WIDTH / 2,
                    parentPos.y() + parent->height() / 2 - HEIGHT / 2,
                    WIDTH, HEIGHT);
    }


    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setAlignment(Qt::AlignLeft);

    QToolButton* addParamBtn = new QToolButton(this);
    addParamBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    addParamBtn->setIcon(QIcon(":/images/edit_add.png"));
    addParamBtn->setIconSize(QSize(64, 64));
    connect(addParamBtn, SIGNAL(clicked()), this, SLOT(addParam()));
    hLayout->addWidget(addParamBtn);

    QToolButton* refreshBtn = new QToolButton(this);
    refreshBtn->setText(tr("Refresh"));
    refreshBtn->setFixedHeight(BTN_SIZE);
    refreshBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(refreshBtn, SIGNAL(clicked()), this, SLOT(refreshParams()));
    hLayout->addWidget(refreshBtn);

    mLayout->addLayout(hLayout);

    addParam();

    /*
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
    */

    setWindowTitle("Manual Monitor");

    updateUI();
}

MonitorManual::~MonitorManual()
{

}

void MonitorManual::paintEvent(QPaintEvent *event)
{

}

void MonitorManual::mousePressEvent(QMouseEvent *event)
{

}

void MonitorManual::mouseReleaseEvent(QMouseEvent *event)
{

}

void MonitorManual::mouseMoveEvent(QMouseEvent *event)
{

}

void MonitorManual::onPlayClicked()
{
    //TODO
}

void MonitorManual::onPauseClicked()
{
    //TODO
}

void MonitorManual::onStopClicked()
{
    //TODO
}

void MonitorManual::setUpdatePeriod(QString period)
{
    /*
    mUpdatePeriod = period.toInt();
    if (!mIsAutoMode)
    {
        return;
    }
*/
    //TODO timer
}

void MonitorManual::setAutoMode()
{
    /*
    mIsAutoMode = true;
    updateUI();
    */
}

void MonitorManual::setManualMode()
{
    /*
    mIsAutoMode = false;
    updateUI();

    mTimer->stop();
    */
}

void MonitorManual::updateUI()
{
    /*
    for (int i = 0, sz = mManualModeWidgets.size(); i < sz;++i)
    {
        mManualModeWidgets[i]->setVisible(!mIsAutoMode);
    }

    for (int i = 0, sz = mAutoModeWidgets.size(); i < sz; ++i)
    {
        mAutoModeWidgets[i]->setVisible(mIsAutoMode);
    }

    mPlot->setVisible(mPlotCheckBox->isChecked() && mIsAutoMode);
*/
    adjustSize();
    update();
}

void MonitorManual::getCurrentValue()
{
    //TODO
}

void MonitorManual::addParam()
{
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setAlignment(Qt::AlignLeft);

    QToolButton* removeParamBtn = new QToolButton(this);
    removeParamBtn->setIcon(QIcon(":/images/edit_remove.png"));
    connect(removeParamBtn, SIGNAL(clicked()), this, SLOT(removeParam()));
    hLayout->addWidget(removeParamBtn);

    QComboBox* comboBox = new QComboBox(this);
    comboBox->addItem("Param 1");
    comboBox->addItem("Param 2");
    comboBox->addItem("Param 3");
    comboBox->addItem("Param 4");
    comboBox->addItem("Param 5");
    comboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    hLayout->addWidget(comboBox);

    QLineEdit* value = new QLineEdit(this);
    value->setReadOnly(true);
    value->setAlignment(Qt::AlignRight);
    value->setText(QString::number(0));
    value->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    //value->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    hLayout->addWidget(value);

    QLabel* activeLabel = new QLabel(this);
    activeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    activeLabel->setText("Units");
    hLayout->addWidget(activeLabel);

    mLayout->addLayout(hLayout);

    mDeleteButtons.push_back(removeParamBtn);
    updateUI();
}

void MonitorManual::refreshParams()
{
    //TODO run the list of params and set text to all QlineEdit
}

void MonitorManual::removeParam()
{
    QObject* sender = QObject::sender();
    if (sender == Q_NULLPTR)
    {
        return;
    }

    for (int i = 0, sz = mDeleteButtons.size(); i < sz; ++i)
    {
        if (sender == mDeleteButtons[i])
        {
            mDeleteButtons.remove(i);
            QLayoutItem* item = mLayout->takeAt(i + 1); // +1 - exclude header of layout with "Add" and "Refresh" buttons
            QLayout* layout = item->layout();

            if (layout)
            {
                QLayoutItem* child = layout->takeAt(0);

                while (child != Q_NULLPTR)
                {
                    if (child->widget())
                    {
                        child->widget()->deleteLater();
                    }

                    child = layout->takeAt(0);
                }
            }

            delete item;
            break;
        }
    }

    updateUI();
}
