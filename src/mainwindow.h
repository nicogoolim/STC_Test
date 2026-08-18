#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class TableModel;
class DatabaseWorker;
class ImportWorker;
class ProgressDialog;
class QThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void importRequested(const QString &filePath);

private slots:
    void importFolder();
    void deleteSelected();
    void editSelected();
    void exportJson(int row);
    void exportCsv(int row);
    void onDataReady(const QList<QList<QVariant>> &rows, const QStringList &headers, const QList<int> &rowIds);
    void onOperationFinished(bool success, const QString &message);
    void onImportReady(bool success, const QList<QList<QVariant>> &rows, const QStringList &headers, const QString &error);
    void onProgressUpdated(int current, int total, const QString &status);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupDatabase();
    void setupImportWorker();
    void refreshData();
    void startImport();
    void showContextMenu(const QPoint &pos);
    void editCell(int row, int column);
    void setupConnections();
    QStringList getModelHeaders() const;
    QList<QVariant> getRowData(int row) const;

    Ui::MainWindow *m_ui;
    TableModel *m_model;
    DatabaseWorker *m_worker;
    QThread *m_workerThread;
    ImportWorker *m_importWorker;
    ProgressDialog *m_progressDialog;
    QString m_tableName;
    bool m_dbReady;
};

#endif // MAINWINDOW_H
