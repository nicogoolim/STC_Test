#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>

class ImportWorker : public QObject
{
    Q_OBJECT

public:
    explicit ImportWorker(QObject *parent = 0);
    ~ImportWorker();

public slots:
    void doImport(const QString &filePath);

signals:
    void importFinished(bool success, const QList<QList<QVariant>> &rows, const QStringList &headers, const QString &error);
    void progressUpdated(int current, int total, const QString &status);
};

#endif // IMPORTWORKER_H