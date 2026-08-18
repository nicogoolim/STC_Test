#include "databaseworker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>

DatabaseWorker::DatabaseWorker(QObject *parent)
    : QObject(parent)
{
    m_isOpen = false;
}

DatabaseWorker::~DatabaseWorker()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseWorker::open(const QString &dbPath)
{
    m_dbPath = dbPath;
    QString connectionName = QString("db_%1").arg((quintptr)this, 0, 16);
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        emit operationFinished(false, tr("Не удалось открыть базу данных: %1").arg(m_db.lastError().text()));
        return false;
    }

    m_isOpen = true;

    emit operationFinished(true, "БД открыта");
    return true;
}

bool DatabaseWorker::createTable(const QString &tableName, const QStringList &headers)
{
    if (!m_isOpen) 
        return false;
    QSqlQuery query(m_db);

    QStringList columns;
    columns.append("id INTEGER PRIMARY KEY AUTOINCREMENT");
    for (const QString &h : headers) {
        QString colName = QString("\"%1\"").arg(h);
        columns.append(QString("%1 TEXT").arg(colName));
    }

    QString sql = QString("CREATE TABLE IF NOT EXISTS %1 (%2)").arg(tableName, columns.join(", "));
    if (!query.exec(sql)) {
        emit operationFinished(false, tr("Не удалось создать таблицу: %1").arg(query.lastError().text()));
        return false;
    }

    emit operationFinished(true, "БД готова");
    return true;
}

bool DatabaseWorker::insertRow(const QString &tableName, const QList<QVariant> &values, const QStringList &headers)
{
    if (!m_isOpen) 
        return false;

    QStringList quotedHeaders;
    for (const QString &h : headers)
        quotedHeaders.append(QString("\"%1\"").arg(h));

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(
        tableName,
        quotedHeaders.join(", "),
        QString("?").repeated(headers.size()).split("", Qt::SkipEmptyParts).join(", "));

    QSqlQuery query(m_db);
    query.prepare(sql);

    for (const QVariant &v : values)
        query.addBindValue(v.toString());

    if (!query.exec()) {
        emit operationFinished(false, tr("Ошибка: %1").arg(query.lastError().text()));
        return false;
    }

    emit operationFinished(true, "Строка вставлена");
    return true;
}

bool DatabaseWorker::updateRow(const QString &tableName, int rowId, const QList<QVariant> &values, const QStringList &headers)
{
    if (!m_isOpen) 
        return false;

    QStringList setClauses;
    for (const QString &h : headers)
        setClauses.append(QString("\"%1\" = ?").arg(h));

    QString sql = QString("UPDATE %1 SET %2 WHERE id = ?").arg(
        tableName,
        setClauses.join(", "));

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &v : values)
        query.addBindValue(v.toString());
    query.addBindValue(rowId);

    if (!query.exec()) {
        emit operationFinished(false, tr("Ошибка: %1").arg(query.lastError().text()));
        return false;
    }
    emit operationFinished(true, "Строка обновлена");
    return true;
}

bool DatabaseWorker::deleteRow(const QString &tableName, int rowId)
{
    if (!m_isOpen) 
        return false;

    QSqlQuery query(m_db);
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(tableName));

    query.addBindValue(rowId);

    if (!query.exec()) {
        emit operationFinished(false, tr("Ошибка удаления: %1").arg(query.lastError().text()));
        return false;
    }

    emit operationFinished(true, "Строка удалена");
    return true;
}

QList<QList<QVariant>> DatabaseWorker::fetchAll(const QString &tableName)
{
    QList<QList<QVariant>> rows;
    QStringList headers;
    QList<int> rowIds;

    if (!m_isOpen)
        return rows;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM " + tableName + " ORDER BY id")) {
        emit operationFinished(false, tr("Ошибка получения данных: %1").arg(query.lastError().text()));
        return rows;
    }

    QSqlRecord record = query.record();
    for (int i = 1; i < record.count(); ++i)
        headers.append(record.fieldName(i));

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 1; i < record.count(); ++i)
            row.append(query.value(i));
        rows.append(row);
        rowIds.append(query.value(0).toInt());
    }

    emit dataReady(rows, headers, rowIds);
    emit operationFinished(true, "Данные загружены");
    return rows;
}

