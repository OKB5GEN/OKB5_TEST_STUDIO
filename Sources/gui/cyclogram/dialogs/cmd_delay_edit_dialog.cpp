#include "Headers/gui/cyclogram/dialogs/cmd_delay_edit_dialog.h"
#include "Headers/logic/commands/cmd_delay.h"
#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/logic/variable_controller.h"
#include "Headers/logger/Logger.h"

#include <QtWidgets>

CmdDelayEditDialog::CmdDelayEditDialog(QWidget * parent):
    QDialog(parent),
    mCommand(Q_NULLPTR),
    mHours(0),
    mMinutes(0),
    mSeconds(0),
    mMSeconds(0)
{
    QGridLayout * mainLayout = new QGridLayout(this);

    QGroupBox* directValueBox = new QGroupBox(tr("Direct value"), this);
    QGridLayout * directValLayout = new QGridLayout(directValueBox);
    mHSpin = addItem(directValLayout, tr("Hours"), 0, 23, SLOT(onHoursChanged(int)));
    mMSpin = addItem(directValLayout, tr("Minutes"), 1, 59, SLOT(onMinutesChanged(int)));
    mSSpin = addItem(directValLayout, tr("Seconds"), 2, 59, SLOT(onSecondsChanged(int)));
    mMSSpin = addItem(directValLayout, tr("Milliseconds"), 3, 999, SLOT(onMilliSecondsChanged(int)));
    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    directValLayout->addItem(spacer, 4, 0, -1, -1);
    directValueBox->setLayout(directValLayout);
    mainLayout->addWidget(directValueBox, 0, 0);

    QGroupBox* variableBox = new QGroupBox(tr("Variable"), this);
    QVBoxLayout* varLayout = new QVBoxLayout(variableBox);
    mVariablesList = new QListWidget(this);
    mVariablesList->setSelectionMode(QAbstractItemView::SingleSelection);
    varLayout->addWidget(mVariablesList);
    variableBox->setLayout(varLayout);
    mainLayout->addWidget(variableBox, 0, 1);

    mConsoleTextWidget = new ConsoleTextWidget(this);
    mainLayout->addWidget(mConsoleTextWidget, 1, 0, 1, 2);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox, 2, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(mainLayout);
    setWindowTitle(tr("Set Delay"));

    adjustSize();
    setFixedSize(sizeHint());
}

CmdDelayEditDialog::~CmdDelayEditDialog()
{

}

void CmdDelayEditDialog::setCommand(CmdDelay* command)
{
    mCommand = command;

    if (!mCommand)
    {
        return;
    }

    mVariablesList->clear();
    auto variablesNames = mCommand->variableController()->variablesData();
    mVariablesList->addItems(variablesNames.keys());

    if (!mCommand->variable().isEmpty())
    {
        auto varList = mVariablesList->findItems(mCommand->variable(), Qt::MatchExactly);
        if (!varList.isEmpty())
        {
            mVariablesList->setItemSelected(varList.front(), true);
        }
    }
    else
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

    mConsoleTextWidget->setCommand(mCommand);
}

void CmdDelayEditDialog::onHoursChanged(int hours)
{
    mHours = hours;
    mVariablesList->clearSelection();
}

void CmdDelayEditDialog::onMinutesChanged(int minutes)
{
    mMinutes = minutes;
    mVariablesList->clearSelection();
}

void CmdDelayEditDialog::onSecondsChanged(int seconds)
{
    mSeconds = seconds;
    mVariablesList->clearSelection();
}

void CmdDelayEditDialog::onMilliSecondsChanged(int mseconds)
{
    mMSeconds = mseconds;
    mVariablesList->clearSelection();
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
    if (!mCommand)
    {
        LOG_ERROR(QString("CmdDelayEditDialog::onAccept: No command!"));
        reject();
        return;
    }

    auto selected = mVariablesList->selectedItems();
    if (selected.isEmpty())
    {
        mCommand->setDelay(mHours, mMinutes, mSeconds, mMSeconds);
    }
    else
    {
        mCommand->setDelay(selected.front()->text());
    }

    mConsoleTextWidget->saveCommand();

    accept();
}
