#include "importer.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

bool Importer::importJson(const QString &filePath, QList<QList<QVariant>> &rows, QStringList &headers)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray())
        return false;

    QJsonArray arr = doc.array();
    if (arr.isEmpty())
        return false;

    QJsonObject firstObj = arr.first().toObject();
    headers = firstObj.keys();

    for (int i = 0; i < arr.size(); ++i) {
        QList<QVariant> row;
        QJsonObject obj = arr.at(i).toObject();
        for (const QString &h : headers)
            row.append(obj.value(h).toVariant());
        rows.append(row);
    }

    return true;
}

bool Importer::importCsv(const QString &filePath, QList<QList<QVariant>> &rows, QStringList &headers)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    in.setCodec("UTF-8");

    headers = in.readLine().split(',');

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',');
        QList<QVariant> row;
        for (const QString &part : parts)
            row.append(part.trimmed());
        rows.append(row);
    }

    file.close();
    return !rows.isEmpty();
}
