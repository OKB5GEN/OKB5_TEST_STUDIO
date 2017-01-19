#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include "Headers/gui/editor_window.h"

#include "Headers/logger/Logger.h"
#include "Headers/logger/FileAppender.h"
#include "Headers/logger/ConsoleAppender.h"

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

    // logging initialization (create console and file outputs)
    ConsoleAppender* consoleAppender = new ConsoleAppender();
    //consoleAppender->setFormat("[%-7l] <%C> %m\n");
    consoleAppender->setFormat("[%{type}] <%{function}> %{message}\n");

    Logger::globalInstance()->registerAppender(consoleAppender);

    FileAppender* fileAppender = new FileAppender("OKB5Studio.log");
    Logger::globalInstance()->registerAppender(fileAppender);

    LOG_INFO("========== APPLICATION STARTED ==========");

    EditorWindow mainWin;
    if (!parser.positionalArguments().isEmpty())
    {
        mainWin.loadFile(parser.positionalArguments().first());
    }

    mainWin.show();
    mainWin.init();
    int result = app.exec();

    LOG_INFO("========== APPLICATION FINISHED ==========");

    return result;
}


