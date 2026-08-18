#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSqlDatabase>

class DatabaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject *parent = 0);
    ~DatabaseWorker();


public:
    bool open(const QString &dbPath);
    bool createTable(const QString &tableName, const QStringList &headers);
    bool insertRow(const QString &tableName, const QList<QVariant> &values, const QStringList &headers);
    bool updateRow(const QString &tableName, int rowId, const QList<QVariant> &values, const QStringList &headers);
    bool deleteRow(const QString &tableName, int rowId);
    QList<QList<QVariant>> fetchAll(const QString &tableName);

signals:
    void operationFinished(bool success, const QString &message);
    void dataReady(const QList<QList<QVariant>> rows, const QStringList headers, const QList<int> rowIds);

private:
    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_isOpen;
};

#endif // DATABASEWORKER_H
