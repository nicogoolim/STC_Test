#include "importworker.h"
#include "importer.h"
#include <QFileInfo>

ImportWorker::ImportWorker(QObject *parent)
    : QObject(parent)
{
}

ImportWorker::~ImportWorker()
{
}

void ImportWorker::doImport(const QString &filePath)
{
    emit progressUpdated(0, 1, QFileInfo(filePath).fileName());

    QList<QList<QVariant>> rows;
    QStringList headers;

    bool success = false;
    if (filePath.endsWith(".json", Qt::CaseInsensitive))
        success = Importer::importJson(filePath, rows, headers);
    else if (filePath.endsWith(".csv", Qt::CaseInsensitive))
        success = Importer::importCsv(filePath, rows, headers);

    if (!success || rows.isEmpty()) {
        emit importFinished(false, rows, headers, "Не удалось импортировать файл");
        return;
    }

    emit progressUpdated(1, 1, "Done");
    emit importFinished(true, rows, headers, QString());
}