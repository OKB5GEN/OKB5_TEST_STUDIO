#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDateTime>
#include <QDir>
#include <QTranslator>

#include "Headers/gui/editor_window.h"
#include "Headers/app_settings.h"

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

//void MyWidget::changeEvent(QEvent *event) //TODO GUI update after locale changed
//{
//    if (event->type() == QEvent::LanguageChange)
//    {
//        titleLabel->setText(tr("Document Title"));
//        ...
//        okPushButton->setText(tr("&OK"));
//    }
//    else
//    {
//        QWidget::changeEvent(event);
//    }
//}

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
    parser.addPositionalArgument("file", "The file to open."); // TODO use this
    parser.process(app);

    createDirs();

    QLocale::setDefault(QLocale::c());

    initializeLogger();

    AppSettings::instance().load();

    //TODO localization on (maybe from command line) as "-l ru -f file_name" for example, guess parser.addPositionalArgument() must be used
    QTranslator translator;
    translator.load("OKB5TestStudio_ru", ":/resources/translations");
    app.installTranslator(&translator);

    LOG_INFO(QString("========== APPLICATION STARTED =========="));

    EditorWindow mainWin;
//    if (!parser.positionalArguments().isEmpty())
//    {
//        mainWin.loadFile(parser.positionalArguments().first(), true);
//    }

    mainWin.show();
    mainWin.onApplicationStart();
    int result = app.exec();

    //AppSettings::instance().save();

    LOG_INFO(QString("========== APPLICATION FINISHED =========="));

    return result;
}
