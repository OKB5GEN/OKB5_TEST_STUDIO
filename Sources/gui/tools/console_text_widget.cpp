#include "Headers/gui/tools/console_text_widget.h"
#include "Headers/logger/Logger.h"
#include "Headers/logic/command.h"

#include <QtWidgets>

ConsoleTextWidget::ConsoleTextWidget(QWidget * parent):
    QGroupBox(parent),
    mCommand(Q_NULLPTR)
{
    setTitle(tr("Console text"));
    QGridLayout* boxLayout = new QGridLayout(this);

    mColorBox = new QComboBox(this);
    mColorBox->installEventFilter(this);
    mTextEdit = new QLineEdit(this);

    boxLayout->addWidget(mTextEdit, 0, 0);
    boxLayout->addWidget(mColorBox, 0, 1);

    setLayout(boxLayout);

    colorsList(); // just for colors list creation

    setColors(mColorBox);

    setMinimumWidth(500);
}

ConsoleTextWidget::~ConsoleTextWidget()
{

}

void ConsoleTextWidget::setColors(QComboBox *box)
{
    box->clear();

    foreach (QString colorName, ConsoleTextWidget::colorsList())
    {
        QColor color(colorName);

        QPixmap pixmap(16,16);
        pixmap.fill(color);
        QIcon icon(pixmap);

        box->addItem(icon, colorName, QVariant(color.rgba()));
    }
}

void ConsoleTextWidget::setCommand(Command* command)
{
    mCommand = command;

    if (!mCommand)
    {
        return;
    }

    QColor color = mCommand->consoleTextColor();
    int index = mColorBox->findData(QVariant(color.rgba()));

    mTextEdit->setText(mCommand->consoleText());

    if (index != -1)
    {
        mColorBox->setCurrentIndex(index);
    }
}

void ConsoleTextWidget::saveCommand()
{
    if (!mCommand)
    {
        return;
    }

    QColor color = QColor::fromRgba(mColorBox->itemData(mColorBox->currentIndex()).toUInt());

    mCommand->setConsoleMessage(mTextEdit->text(), color.rgba());
}

bool ConsoleTextWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(obj))
    {
        return true; // do not process wheel events if combo box is not "expanded/opened"
    }
    else
    {
        return QObject::eventFilter(obj, event); // standard event processing
    }
}

const QStringList& ConsoleTextWidget::colorsList()
{
    static QStringList colors;
    if (colors.empty())
    {
        colors = QColor::colorNames();

        // Подборка если фон виджета светлый

        // Не видно на белом фоне полностью или практически и надо напрягать зрение (убираем 100%)
        colors.removeAll("aliceblue");
        colors.removeAll("antiquewhite");
        colors.removeAll("azure");
        colors.removeAll("beige");
        colors.removeAll("bisque");
        colors.removeAll("blanchedalmond");
        colors.removeAll("cornsilk");
        colors.removeAll("floralwhite");
        colors.removeAll("gainsboro");
        colors.removeAll("ghostwhite");
        colors.removeAll("honeydew");
        colors.removeAll("ivory");
        colors.removeAll("khaki");
        colors.removeAll("lavender");
        colors.removeAll("lavenderblush");
        colors.removeAll("lemonchiffon");
        colors.removeAll("lightcyan");
        colors.removeAll("lightgoldenrodyellow");
        colors.removeAll("lightgray");
        colors.removeAll("lightgreen");
        colors.removeAll("lightgrey");
        colors.removeAll("lightpink");
        colors.removeAll("lightyellow");
        colors.removeAll("linen");
        colors.removeAll("mintcream");
        colors.removeAll("mistyrose");
        colors.removeAll("moccasin");
        colors.removeAll("navajowhite");
        colors.removeAll("oldlace");
        colors.removeAll("palegoldenrod");
        colors.removeAll("palegreen");
        colors.removeAll("paleturquoise");
        colors.removeAll("papayawhip");
        colors.removeAll("peachpuff");
        colors.removeAll("seashell");
        colors.removeAll("snow");
        colors.removeAll("transparent");
        colors.removeAll("wheat");
        colors.removeAll("white");
        colors.removeAll("whitesmoke");

        // Кислотные, напрягающие зрение (убираем, но какие-то может и можно оставить)
        colors.removeAll("aqua");
        colors.removeAll("aquamarine");
        colors.removeAll("chartreuse");
        colors.removeAll("cyan");
        colors.removeAll("darkturquoise");
        colors.removeAll("deepskyblue");
        colors.removeAll("fuchsia");
        colors.removeAll("gold");
        colors.removeAll("greenyellow");
        colors.removeAll("lawngreen");
        colors.removeAll("lime");
        colors.removeAll("magenta");
        colors.removeAll("mediumspringgreen");
        colors.removeAll("springgreen");
        colors.removeAll("turquoise");
        colors.removeAll("violet");
        colors.removeAll("yellow");

        // Тупо дублирующиеся цвета из-за написания "серого" gray/grey
        colors.removeAll("darkgray");
        colors.removeAll("darkslategray");
        colors.removeAll("dimgray");
        colors.removeAll("gray");
        colors.removeAll("lightslategray");
        colors.removeAll("slategray");

        // Тускловатые (может зайти для некоторых типов сообщений)
        colors.removeAll("burlywood");
        colors.removeAll("cadetblue");
        colors.removeAll("darkgrey");
        colors.removeAll("darkkhaki");
        colors.removeAll("darksalmon");
        colors.removeAll("darkseagreen");
        colors.removeAll("hotpink");
        colors.removeAll("lightblue");
        colors.removeAll("lightcoral");
        colors.removeAll("lightsalmon");
        colors.removeAll("lightskyblue");
        colors.removeAll("lightsteelblue");
        colors.removeAll("mediumaquamarine");
        colors.removeAll("mediumorchid");
        colors.removeAll("mediumpurple");
        colors.removeAll("mediumturquoise");
        colors.removeAll("palevioletred");
        colors.removeAll("pink");
        colors.removeAll("plum");
        colors.removeAll("powderblue");
        colors.removeAll("rosybrown");
        colors.removeAll("salmon");
        colors.removeAll("sandybrown");
        colors.removeAll("silver");
        colors.removeAll("skyblue");
        colors.removeAll("tan");
        colors.removeAll("thistle");
        colors.removeAll("yellowgreen");


        // Нормальные цвета
        //    black	rgb( 0, 0, 0)
        //    blue	rgb( 0, 0, 255)
        //    blueviolet	rgb(138, 43, 226)
        //    brown	rgb(165, 42, 42)
        //    chocolate	rgb(210, 105, 30)
        //    coral	rgb(255, 127, 80)
        //    cornflowerblue	rgb(100, 149, 237)
        //    crimson	rgb(220, 20, 60)
        //    darkblue	rgb( 0, 0, 139)
        //    darkcyan	rgb( 0, 139, 139)
        //    darkgoldenrod	rgb(184, 134, 11)
        //    darkgreen	rgb( 0, 100, 0)
        //    darkmagenta	rgb(139, 0, 139)
        //    darkolivegreen	rgb( 85, 107, 47)
        //    darkorange	rgb(255, 140, 0)
        //    darkorchid	rgb(153, 50, 204)
        //    darkred	rgb(139, 0, 0)
        //    darkslateblue	rgb( 72, 61, 139)
        //    darkslategrey	rgb( 47, 79, 79)
        //    darkviolet	rgb(148, 0, 211)
        //    deeppink	rgb(255, 20, 147)
        //    dimgrey	rgb(105, 105, 105)
        //    dodgerblue	rgb( 30, 144, 255)
        //    firebrick	rgb(178, 34, 34)
        //    forestgreen	rgb( 34, 139, 34)
        //    goldenrod	rgb(218, 165, 32)
        //    grey	rgb(128, 128, 128)
        //    green	rgb( 0, 128, 0)
        //    indianred	rgb(205, 92, 92)
        //    indigo	rgb( 75, 0, 130)
        //    lightseagreen	rgb( 32, 178, 170)
        //    lightslategrey	rgb(119, 136, 153)
        //    limegreen	rgb( 50, 205, 50)
        //    maroon	rgb(128, 0, 0)
        //    mediumblue	rgb( 0, 0, 205)
        //    mediumseagreen	rgb( 60, 179, 113)
        //    mediumslateblue	rgb(123, 104, 238)
        //    mediumvioletred	rgb(199, 21, 133)
        //    midnightblue	rgb( 25, 25, 112)
        //    navy	rgb( 0, 0, 128)
        //    olive	rgb(128, 128, 0)
        //    olivedrab	rgb(107, 142, 35)
        //    orange	rgb(255, 165, 0)
        //    orangered	rgb(255, 69, 0)
        //    orchid	rgb(218, 112, 214)
        //    peru	rgb(205, 133, 63)
        //    purple	rgb(128, 0, 128)
        //    red	rgb(255, 0, 0)
        //    royalblue	rgb( 65, 105, 225)
        //    saddlebrown	rgb(139, 69, 19)
        //    seagreen	rgb( 46, 139, 87)
        //    sienna	rgb(160, 82, 45)
        //    slateblue	rgb(106, 90, 205)
        //    slategrey	rgb(112, 128, 144)
        //    steelblue	rgb( 70, 130, 180)
        //    teal	rgb( 0, 128, 128)
        //    tomato	rgb(255, 99, 71)

//            // "mainstream" colors
//            colors.append("black");
//            colors.append("limegreen");
//            colors.append("crimson");
//            colors.append("orange");
//            colors.append("blue");
//            colors.append("blueviolet");

//            // some custom colors
//            colors.append("silver");
//            colors.append("cadetblue");
//            colors.append("maroon");
//            colors.append("cornflowerblue");
    }

    return colors;
}
