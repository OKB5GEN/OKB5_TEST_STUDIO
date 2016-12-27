#include "Headers/gui/mainwindow.h"
#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include "Headers/gui/editor_window.h"

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

    QLocale::setDefault(QLocale::c());

    EditorWindow mainWin;
    if (!parser.positionalArguments().isEmpty())
    {
        mainWin.loadFile(parser.positionalArguments().first());
    }

    mainWin.show();
    mainWin.init();
    return app.exec();
}

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


