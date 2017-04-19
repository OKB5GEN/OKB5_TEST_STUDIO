#ifndef CYCLOGRAM_MANAGER_H
#define CYCLOGRAM_MANAGER_H

#include <QObject>
#include <QSet>

class Cyclogram;

class CyclogramManager: public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<Cyclogram> createCyclogram(const QString& fileName = QString(), bool* ok = Q_NULLPTR);

    static void clear();
    static void removeCyclogram(QSharedPointer<Cyclogram> cyclogram);

private:
    static QSharedPointer<Cyclogram> createDefault();

    static QSet< QSharedPointer<Cyclogram> > smCyclograms;
};
#endif // CYCLOGRAM_MANAGER_H
