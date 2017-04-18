#include "Headers/logic/cyclogram_manager.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"

#include <QSharedPointer>
#include <QFile>
#include <QDir>

QMap< QString, QSharedPointer<Cyclogram> > CyclogramManager::smOpenedCyclograms;
QSet< QSharedPointer<Cyclogram> > CyclogramManager::smDefaultCyclograms;

CyclogramManager::CyclogramManager()
{

}

QSharedPointer<Cyclogram> CyclogramManager::loadFromFile(const QString& fileName, bool* ok)
{
    auto it = smOpenedCyclograms.find(fileName);
    if (it != smOpenedCyclograms.end())
    {
        if (ok)
        {
            *ok = true;
        }

        return it.value();
    }

    Cyclogram* cyclogram = new Cyclogram(Q_NULLPTR);
    QSharedPointer<Cyclogram> p(cyclogram);

    // pointer "registration" must be before file loading to prevent stack overflow
    // due to recursive file loading (when cyclogram has file link to itself in some of included subprograms)
    smOpenedCyclograms[fileName] = p;

    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot open file %1: %2. Create default cyclogram").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        p->createDefault();
        if (ok)
        {
            *ok = false;
        }

        return p;
    }

    FileReader reader(p);
    if (!reader.read(&file))
    {
        LOG_ERROR(QString("Parse error in file %1: %2. Create default cyclogram").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        p->createDefault();
        if (ok)
        {
            *ok = false;
        }

        return p;
    }

    if (ok)
    {
        *ok = true;
    }

    return p;
}

void CyclogramManager::clear()
{
    smOpenedCyclograms.clear();
    smDefaultCyclograms.clear();
}

QSharedPointer<Cyclogram> CyclogramManager::createDefaultCyclogram()
{
    Cyclogram* cyclogram = new Cyclogram(Q_NULLPTR);
    QSharedPointer<Cyclogram> p(cyclogram);
    smDefaultCyclograms.insert(p);
    return p;
}

void CyclogramManager::onCyclogramSaved(QSharedPointer<Cyclogram> cyclogram, const QString& fileName)
{
    smOpenedCyclograms[fileName] = cyclogram;
    smDefaultCyclograms.remove(cyclogram);
}

void CyclogramManager::removeDefaultCyclogram(QSharedPointer<Cyclogram> cyclogram)
{
    smDefaultCyclograms.remove(cyclogram);
}
