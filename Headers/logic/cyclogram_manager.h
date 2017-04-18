#ifndef CYCLOGRAM_MANAGER_H
#define CYCLOGRAM_MANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>

class Cyclogram;

class CyclogramManager: public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<Cyclogram> loadFromFile(const QString& fileName, bool* ok = Q_NULLPTR);
    static void clear();
    static QSharedPointer<Cyclogram> createDefaultCyclogram();
    static void onCyclogramSaved(QSharedPointer<Cyclogram> cyclogram, const QString& fileName);
    static void removeDefaultCyclogram(QSharedPointer<Cyclogram> cyclogram);

private:
    CyclogramManager();
    static QMap< QString, QSharedPointer<Cyclogram> > smOpenedCyclograms;
    static QSet< QSharedPointer<Cyclogram> > smDefaultCyclograms;

};
#endif // CYCLOGRAM_MANAGER_H
