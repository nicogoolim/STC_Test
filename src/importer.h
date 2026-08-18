#ifndef IMPORTER_H
#define IMPORTER_H

#include <QString>
#include <QList>
#include <QVariant>

class Importer
{
public:
    static bool importJson(const QString &filePath, QList<QList<QVariant>> &rows, QStringList &headers);
    static bool importCsv(const QString &filePath, QList<QList<QVariant>> &rows, QStringList &headers);
};

#endif // IMPORTER_H
