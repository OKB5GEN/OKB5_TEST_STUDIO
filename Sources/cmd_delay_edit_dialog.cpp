#include <QtWidgets>

#include "Headers/cmd_delay_edit_dialog.h"

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;
}

CmdDelayEditDialog::CmdDelayEditDialog(QWidget * parent):
    QDialog(parent)
{
    if (parent != Q_NULLPTR)
    {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        setGeometry(parentPos.x() + parent->width() / 2 - WIDTH / 2,
                    parentPos.y() + parent->height() / 2 - HEIGHT / 2,
                    WIDTH, HEIGHT);
    }


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
        /*
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
        */

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
        //timeEdit->setValidator(new QIntValidator(MIN_UPDATE_INTERVAL, MAX_UPDATE_INTERVAL, this));
        //timeEdit->setText(QString::number(MIN_UPDATE_INTERVAL));
        timeEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(timeEdit);

        connect(timeEdit, SIGNAL(textChanged(QString)), this, SLOT(setUpdatePeriod(QString)));

        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(true);
        activeBox->setText("Show plot");
        activeBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        connect(activeBox, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
        hLayout->addWidget(activeBox);

        //mPlotCheckBox = activeBox;

        hLayout->addStretch();
        vLayout->addLayout(hLayout);
    }

    vLayout->addStretch();

    setLayout(vLayout);
    setWindowTitle("Edit Delay Command");
}

CmdDelayEditDialog::~CmdDelayEditDialog()
{

}

void CmdDelayEditDialog::setCommand(CmdDelay* command)
{

}
