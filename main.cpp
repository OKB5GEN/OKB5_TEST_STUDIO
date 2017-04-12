#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDateTime>
#include <QDir>

#include "Headers/gui/editor_window.h"

#include "Headers/logger/Logger.h"
#include "Headers/logger/FileAppender.h"
#include "Headers/logger/ConsoleAppender.h"

namespace
{
    static const QString REPORTS_DIR = "reports";
    static const QString LOGS_DIR = "logs";
    static const QString CYCLOGRAMS_DIR = "cyclograms";
}

void initializeLogger()
{
    // create console output
    ConsoleAppender* consoleAppender = new ConsoleAppender();
    //consoleAppender->setFormat("[%{type}] <%{function}> %{message}\n");
    consoleAppender->setFormat("%{time} [%{type}] %{message}\n");
    Logger::globalInstance()->registerAppender(consoleAppender);

    // create file output
    QString appStartTime = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_HH_mm_ss");
    QString fileBaseName = LOGS_DIR + "/OKB5TS_";
    QString fileExtenstion = ".log";

    FileAppender* fileAppender = new FileAppender(fileBaseName + appStartTime + fileExtenstion);
    fileAppender->setFormat("%{time} [%{type}] %{message}\n");
    Logger::globalInstance()->registerAppender(fileAppender);
}

void createDirs()
{
    QList<QString> dirs;
    dirs.append(REPORTS_DIR);
    dirs.append(LOGS_DIR);
    dirs.append(CYCLOGRAMS_DIR);

    QDir currentDir = QDir::current();

    for (auto it = dirs.begin(); it != dirs.end(); ++it)
    {
        if (!currentDir.exists(*it))
        {
            currentDir.mkdir(*it);
        }
    }
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

    createDirs();

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


