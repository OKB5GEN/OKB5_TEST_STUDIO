#ifndef CYCLOGRAM_MANAGER_H
#define CYCLOGRAM_MANAGER_H

#include <QObject>
#include <QMap>

class Cyclogram;

class CyclogramManager: public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<Cyclogram> loadFromFile(const QString& fileName);
    static void clear();

private:
    CyclogramManager();
    static QMap< QString, QSharedPointer<Cyclogram> > smOpenedCyclograms;

};
#endif // CYCLOGRAM_MANAGER_H
