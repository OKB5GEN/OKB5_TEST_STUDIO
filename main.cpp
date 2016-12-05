#include "mainwindow.h"
#include <QApplication>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "Headers/editor_window.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("OKB5");
    QCoreApplication::setApplicationName("Cyclogram Editor");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    EditorWindow mainWin;
    if (!parser.positionalArguments().isEmpty())
    {
        mainWin.loadFile(parser.positionalArguments().first());
    }

    mainWin.show();
    return app.exec();
}

/*
// basic drawing
#include "window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(basicdrawing);

    QApplication app(argc, argv);
    Window window;
    window.show();
    return app.exec();
}
*/
/*
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.setWindowFlags(Qt::Dialog);
    //w.setFixedSize(QSize(1130, 630));
    //w.showMaximized();

    w.show();

    return a.exec();
}
*/

/*
#include <QtWidgets>

#include "sortingbox.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(tooltips);

    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    SortingBox sortingBox;
    sortingBox.show();
    return app.exec();
}

*/
