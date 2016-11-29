#include <QtWidgets>

#include "Headers/cmd_delay_edit_dialog.h"
#include "Headers/commands/cmd_delay.h"

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;
}

CmdDelayEditDialog::CmdDelayEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR),
    mHours(0),
    mMinutes(0),
    mSeconds(0),
    mMSeconds(0)
{
    if (parent != Q_NULLPTR)
    {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        setGeometry(parentPos.x() + parent->width() / 2 - WIDTH / 2,
                    parentPos.y() + parent->height() / 2 - HEIGHT / 2,
                    WIDTH, HEIGHT);
    }

    QGridLayout * layout = new QGridLayout(this);

    mHSpin = addItem(layout, tr("Hours"), 0, 23, SLOT(onHoursChanged(int)));
    mMSpin = addItem(layout, tr("Minutes"), 1, 59, SLOT(onMinutesChanged(int)));
    mSSpin = addItem(layout, tr("Seconds"), 2, 59, SLOT(onSecondsChanged(int)));
    mMSSpin = addItem(layout, tr("Milliseconds"), 3, 999, SLOT(onMilliSecondsChanged(int)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 4, 2);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);
    setWindowTitle("Edit Delay Command");

    adjustSize();
    setFixedSize(sizeHint());
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

CmdDelayEditDialog::~CmdDelayEditDialog()
{

}

void CmdDelayEditDialog::setCommand(CmdDelay* command)
{
    mCommand = command;

    if (mCommand)
    {
        int delay = mCommand->delay();
        mHours = delay / 3600 / 1000;
        delay -= mHours * 3600 * 1000;
        mMinutes = delay / 60 / 1000;
        delay -= mMinutes * 60 * 1000;
        mSeconds = delay / 1000;
        delay -= mSeconds * 1000;
        mMSeconds = delay;

        mHSpin->setValue(mHours);
        mMSpin->setValue(mMinutes);
        mSSpin->setValue(mSeconds);
        mMSSpin->setValue(mMSeconds);
    }
}

void CmdDelayEditDialog::onHoursChanged(int hours)
{
    mHours = hours;
}

void CmdDelayEditDialog::onMinutesChanged(int minutes)
{
    mMinutes = minutes;
}

void CmdDelayEditDialog::onSecondsChanged(int seconds)
{
    mSeconds = seconds;
}

void CmdDelayEditDialog::onMilliSecondsChanged(int mseconds)
{
    mMSeconds = mseconds;
}

QSpinBox* CmdDelayEditDialog::addItem(QGridLayout* layout, const QString& text, int row, int max, const char* onChange)
{
    int column = 0;

    QLabel* label = new QLabel(this);
    label->setText(text);
    layout->addWidget(label, row, column);

    QSpinBox* spinBox = new QSpinBox(this);
    spinBox->setRange(0, max);
    spinBox->setValue(0);
    spinBox->setFixedWidth(50);
    layout->addWidget(spinBox, row, column + 1);

    QSlider* slider = new QSlider(this);
    slider->setRange(0, max);
    slider->setValue(0);
    slider->setOrientation(Qt::Horizontal);
    layout->addWidget(slider, row, column + 2);

    connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
    connect(slider, SIGNAL(valueChanged(int)), this, onChange);
    connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
    connect(spinBox, SIGNAL(valueChanged(int)), this, onChange);

    return spinBox;
}

void CmdDelayEditDialog::onAccept()
{
    if (mCommand)
    {
        mCommand->setDelay(mHours, mMinutes, mSeconds, mMSeconds);
    }

    accept();
}
