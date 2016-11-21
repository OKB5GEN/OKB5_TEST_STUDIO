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
