#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDateTime>

#include "Headers/gui/editor_window.h"

#include "Headers/logger/Logger.h"
#include "Headers/logger/FileAppender.h"
#include "Headers/logger/ConsoleAppender.h"

void initializeLogger()
{
    // create console output
    ConsoleAppender* consoleAppender = new ConsoleAppender();
    consoleAppender->setFormat("[%{type}] <%{function}> %{message}\n");
    Logger::globalInstance()->registerAppender(consoleAppender);

    // create file output
    QString appStartTime = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_HH_mm_ss");
    QString fileBaseName = "OKB5TS_";
    QString fileExtenstion = ".log";

    FileAppender* fileAppender = new FileAppender(fileBaseName + appStartTime + fileExtenstion);
    Logger::globalInstance()->registerAppender(fileAppender);
}

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

    initializeLogger();

    LOG_INFO("========== APPLICATION STARTED ==========");

    EditorWindow mainWin;
    if (!parser.positionalArguments().isEmpty())
    {
        mainWin.loadFile(parser.positionalArguments().first());
    }

    mainWin.show();
    mainWin.onApplicationStart();
    int result = app.exec();

    LOG_INFO("========== APPLICATION FINISHED ==========");

    return result;
}


