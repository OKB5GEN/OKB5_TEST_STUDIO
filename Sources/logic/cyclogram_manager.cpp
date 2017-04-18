#include "Headers/logic/cyclogram_manager.h"
#include "Headers/logic/cyclogram.h"
#include "Headers/logger/Logger.h"
#include "Headers/file_reader.h"

#include <QSharedPointer>
#include <QFile>
#include <QDir>

QMap< QString, QSharedPointer<Cyclogram> > CyclogramManager::smOpenedCyclograms;

CyclogramManager::CyclogramManager()
{

}

QSharedPointer<Cyclogram> CyclogramManager::loadFromFile(const QString& fileName)
{
    auto it = smOpenedCyclograms.find(fileName);
    if (it != smOpenedCyclograms.end())
    {
        return it.value();
    }

    Cyclogram* cyclogram = new Cyclogram(Q_NULLPTR);
    QSharedPointer<Cyclogram> p(cyclogram);
    smOpenedCyclograms[fileName] = p;

    QFile file(fileName);
    FileReader reader(cyclogram);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR(QString("Cannot open file %1: %2. Create default cyclogram").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        p->createDefault();
        //smOpenedCyclograms.remove(fileName);
        return p;
    }

    if (!reader.read(&file))
    {
        LOG_ERROR(QString("Parse error in file %1: %2. Create default cyclogram").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        p->createDefault();
        //smOpenedCyclograms.remove(fileName);
        return p;
    }

    return p;
}

void CyclogramManager::clear()
{
    smOpenedCyclograms.clear();
}
