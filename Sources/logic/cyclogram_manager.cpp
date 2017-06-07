#include "Headers/logic/cyclogram_manager.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"

#include <QSharedPointer>
#include <QFile>
#include <QDir>

QSet< QSharedPointer<Cyclogram> > CyclogramManager::smCyclograms;

QSharedPointer<Cyclogram> CyclogramManager::createCyclogram(const QString& fileName, bool* ok)
{
    if (fileName.isEmpty())
    {
        if (ok)
        {
            *ok = true;
        }

        return createDefault();
    }

    LOG_INFO(QString("Loading '%1' cyclogram file...").arg(QDir::toNativeSeparators(fileName)));

    static QSet<QString> sLoadingFiles;

    // 1. Check file is already loading to prevent recursive files loading (which tends to stack overflow)
    auto itLoading = sLoadingFiles.find(fileName);
    if (itLoading != sLoadingFiles.end())
    {
        LOG_ERROR(QString("Recursive '%1' file loading detected! Skip loading, creating default cyclogram").arg(QDir::toNativeSeparators(fileName)));
        if (ok)
        {
            *ok = false;
        }

        return createDefault();
    }

    // 2. If file is not loading, add file name to loading list, marking that this file loading started
    sLoadingFiles.insert(fileName);

    // 3. Create cyclogram object and
    QSharedPointer<Cyclogram> p(new Cyclogram(Q_NULLPTR));
    smCyclograms.insert(p);

    // 4. Open file for reading
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot open file %1: %2. Creating default cyclogram").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        p->createDefault();
        if (ok)
        {
            *ok = false;
        }

        return p;
    }

    // 5. Read file contents to cyclogram
    FileReader reader(p);
    if (!reader.read(&file))
    {
        LOG_ERROR(QString("Parse error in file %1: %2. Creating default cyclogram").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        p->createDefault();
        if (ok)
        {
            *ok = false;
        }

        return p;
    }

    // 6. Cyclogram successfully loaded from file, erase file name from loading files list
    sLoadingFiles.remove(fileName); // file loaded
    if (ok)
    {
        *ok = true;
    }

    LOG_INFO(QString("'%1' cyclogram file loaded").arg(QDir::toNativeSeparators(fileName)));
    return p;
}

void CyclogramManager::clear()
{
    smCyclograms.clear();
}

QSharedPointer<Cyclogram> CyclogramManager::createDefault()
{
    QSharedPointer<Cyclogram> p(new Cyclogram(Q_NULLPTR));
    p->createDefault();
    smCyclograms.insert(p);
    return p;
}

void CyclogramManager::removeCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    if (cyclogram.isNull())
    {
        return;
    }

    smCyclograms.remove(cyclogram);
}
