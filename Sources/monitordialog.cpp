#include "Headers/monitordialog.h"
#include "qcustomplot.h"

//#include "Headers/sortingbox.h"

//#include <QDialogButtonBox>
//#include <QGridLayout>
//#include <QComboBox>
//#include <QLineEdit>
#include <QtWidgets>

namespace
{
    static const qreal WIDTH = 500;
    static const qreal HEIGHT = 300;
}
MonitorDialog::MonitorDialog(QWidget * parent):
    QDialog(parent)
{
    if (parent != Q_NULLPTR)
    {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        setGeometry(parentPos.x() + parent->width() / 2 - WIDTH / 2,
                    parentPos.y() + parent->height() / 2 - HEIGHT / 2,
                    WIDTH, HEIGHT);
    }

    // ver 1
    /*
    QGridLayout * layout = new QGridLayout(this);

    // value select checkbox
    QComboBox* comboBox = new QComboBox(this);
    comboBox->addItem("Current voltage");
    comboBox->addItem("Temperature 1");
    comboBox->addItem("Temperature 2");
    layout->addWidget(comboBox, 0, 0, 1, 2);

    // active checkbox
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    QCheckBox* activeBox = new QCheckBox(this);
    activeBox->setChecked(false);
    hLayout->addWidget(activeBox);
    QLabel* activeCaption = new QLabel(this);
    activeCaption->setText("Active");
    activeCaption->setAlignment(Qt::AlignLeft);
    activeCaption->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hLayout->addWidget(activeCaption);
    layout->addLayout(hLayout, 0, 3, 1, 1);

    // current value
    QLabel* caption = new QLabel(this);
    caption->setText("Current value");
    caption->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(caption, 1, 0, 1, 1);
    QLabel* value = new QLabel(this);
    value->setText(QString::number(100));
    value->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    value->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(value, 1, 1, 1, 1);

    QCustomPlot* plot = new QCustomPlot(this);
    plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
    //plot->setVisible(false);
    layout->addWidget(plot, 2, 0, 12, 4);
    */

    // ver 2
    QGridLayout * layout = new QGridLayout(this);

    QComboBox* comboBox = new QComboBox(this);
    comboBox->addItem("Select value to monitor...");
    comboBox->addItem("Current voltage");
    comboBox->addItem("Temperature 1");
    comboBox->addItem("Temperature 2");
    comboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(comboBox, 0, 0, 1, 2);

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QToolButton* playBtn = new QToolButton(this);
        hLayout->addWidget(playBtn);
        QToolButton* pauseBtn = new QToolButton(this);
        hLayout->addWidget(pauseBtn);
        QToolButton* stopBtn = new QToolButton(this);
        hLayout->addWidget(stopBtn);
        QToolButton* restartBtn = new QToolButton(this);
        hLayout->addWidget(restartBtn);

        hLayout->addStretch();
        layout->addLayout(hLayout, 1, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 2, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 3, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 4, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 5, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        hLayout->setAlignment(Qt::AlignLeft);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 6, 0, 1, 4);
    }

    {
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        QCheckBox* activeBox = new QCheckBox(this);
        activeBox->setChecked(false);
        hLayout->addWidget(activeBox);
        QLabel* activeCaption = new QLabel(this);
        activeCaption->setText("Caption");
        activeCaption->setAlignment(Qt::AlignLeft);
        activeCaption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        activeCaption->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        hLayout->addWidget(activeCaption);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(lineEdit);
        QLabel* uintsName = new QLabel(this);
        uintsName->setText(", Volts");
        uintsName->setAlignment(Qt::AlignLeft);
        uintsName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hLayout->addWidget(uintsName);

        hLayout->addStretch();
        layout->addLayout(hLayout, 7, 0, 1, 4);
    }


    QCustomPlot* plot = new QCustomPlot(this);
    plot->setMinimumSize(QSize(WIDTH * 0.95, HEIGHT * 0.7));
    //plot->setVisible(false);
    layout->addWidget(plot, 8, 0, 12, 4);


    /*
    mLineEdit = new QLineEdit(this);
    mLineEdit->setText("Default value");

    //connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel , Qt::Horizontal, this);
    layout->addWidget(buttonBox, 1, 0);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    */

    //mShapeType = ShapeTypes::ACTION;

    setLayout(layout);
    setWindowTitle("Monitor");

    adjustSize(); // to resize window to fit its contents
}

MonitorDialog::~MonitorDialog()
{

}
/*
ShapeTypes MonitorDialog::shapeType() const
{
    return mShapeType;
}
*/
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

/*
QString MonitorDialog::text() const
{
    return mLineEdit->text();
}

void MonitorDialog::setText(const QString& text)
{
    mLineEdit->setText(text);
}
*/
