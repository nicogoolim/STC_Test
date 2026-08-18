#include "mainwindow.h"
#include "tablemodel.h"
#include "databaseworker.h"
#include "importworker.h"
#include "progressdialog.h"
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QThread>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::MainWindow),
      m_model(nullptr),
      m_worker(nullptr),
      m_workerThread(nullptr),
      m_importWorker(nullptr),
      m_progressDialog(nullptr),
      m_dbReady(false)
{
    m_ui->setupUi(this);
    m_tableName = "imported_data";
    setupConnections();
    setupDatabase();
    setupImportWorker();
}

void MainWindow::setupImportWorker()
{
    m_importWorker = new ImportWorker(this);

    connect(this, &MainWindow::importRequested, m_importWorker, &ImportWorker::doImport);
    connect(m_importWorker, &ImportWorker::importFinished, this, &MainWindow::onImportReady);
    connect(m_importWorker, &ImportWorker::progressUpdated, this, &MainWindow::onProgressUpdated);
}

void MainWindow::onProgressUpdated(int current, int total, const QString &status)
{
    if (!m_progressDialog) {
        m_progressDialog = new ProgressDialog(this);
    }
    m_progressDialog->setProgress(current, total, status);
    if (!m_progressDialog->isVisible()) {
        m_progressDialog->show();
    }
}

MainWindow::~MainWindow()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete m_ui;
}

void MainWindow::setupConnections()
{
    m_model = new TableModel(this);
    m_ui->tableView->setModel(m_model);
    m_ui->tableView->horizontalHeader()->setStretchLastSection(true);
    m_ui->tableView->viewport()->installEventFilter(this);

    statusBar()->showMessage(tr("Готово"));

    connect(m_ui->actionImport, &QAction::triggered, this, &MainWindow::importFolder);
    connect(m_ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteSelected);
    connect(m_ui->actionEdit, &QAction::triggered, this, &MainWindow::editSelected);
}

QStringList MainWindow::getModelHeaders() const
{
    QStringList headers;
    for (int col = 0; col < m_model->columnCount(); ++col)
        headers.append(m_model->headerData(col, Qt::Horizontal).toString());
    return headers;
}

QList<QVariant> MainWindow::getRowData(int row) const
{
    QList<QVariant> data;
    for (int col = 0; col < m_model->columnCount(); ++col)
        data.append(m_model->data(m_model->index(row, col), Qt::DisplayRole));
    return data;
}

void MainWindow::setupDatabase()
{
    m_workerThread = new QThread(this);
    m_worker = new DatabaseWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, this, [this]() {
        QString dbPath = QApplication::applicationDirPath() + "/data.db";
        m_worker->open(dbPath);
    });

    connect(m_worker, &DatabaseWorker::operationFinished, this, &MainWindow::onOperationFinished);
    connect(m_worker, &DatabaseWorker::dataReady, this, &MainWindow::onDataReady);

    m_workerThread->start();
}

void MainWindow::refreshData()
{
    if (m_dbReady)
        m_worker->fetchAll(m_tableName);
}

void MainWindow::startImport()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Выберите файл JSON или CSV"),
        QString(),
        tr("Файлы данных (*.json *.csv);;Все файлы (*.*)"));

    if (filePath.isEmpty())
        return;

    QMetaObject::invokeMethod(m_importWorker, "doImport", Qt::QueuedConnection, Q_ARG(QString, filePath));
}

void MainWindow::importFolder()
{
    if (!m_dbReady)
        return;

    startImport();
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = m_ui->tableView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);

    QAction *editAction = menu.addAction(tr("Редактировать"));
    connect(editAction, &QAction::triggered, this, [this, index]() {
        editCell(index.row(), index.column());
    });

    QAction *deleteAction = menu.addAction(tr("Удалить"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);

    QMenu *exportMenu = menu.addMenu(tr("Экспорт"));

    QAction *exportJsonAction = exportMenu->addAction(tr("Экспорт JSON"));
    connect(exportJsonAction, &QAction::triggered, this, [this, index]() {
        exportJson(index.row());
    });

    QAction *exportCsvAction = exportMenu->addAction(tr("Экспорт CSV"));
    connect(exportCsvAction, &QAction::triggered, this, [this, index]() {
        exportCsv(index.row());
    });

    menu.exec(m_ui->tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::editCell(int row, int column)
{
    if (!m_dbReady)
        return;

    int rowId = m_model->rowId(row);
    if (rowId < 0)
        return;

    QModelIndex index = m_model->index(row, column);
    QString currentValue = m_model->data(index, Qt::EditRole).toString();
    QString header = m_model->headerData(column, Qt::Horizontal).toString();

    bool ok;
    QString newValue = QInputDialog::getText(this, tr("Редактирование %1").arg(header), header, QLineEdit::Normal, currentValue, &ok);

    if (!ok)
        return;

    QList<QVariant> rowData;
    QStringList headers;
    for (int c = 0; c < m_model->columnCount(); ++c) {
        headers.append(m_model->headerData(c, Qt::Horizontal).toString());
        if (c == column)
            rowData.append(newValue);
        else
            rowData.append(m_model->data(m_model->index(row, c), Qt::DisplayRole));
    }

    m_worker->updateRow(m_tableName, rowId, rowData, headers);
    refreshData();
}

void MainWindow::editSelected()
{
    QModelIndex index = m_ui->tableView->currentIndex();
    if (!index.isValid())
        return;
    editCell(index.row(), index.column());
}

void MainWindow::exportJson(int row)
{
    if (row < 0 || row >= m_model->rowCount()) {
        QMessageBox::information(this, tr("Информация"), tr("Выберите строку для экспорта."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, tr("Экспорт JSON"), QString(), tr("Файлы JSON (*.json)"));
    if (filePath.isEmpty()) return;

    QStringList headers = getModelHeaders();
    QList<QVariant> values = getRowData(row);

    QJsonObject rootObj;
    for (int col = 0; col < values.size(); ++col)
        rootObj[headers.at(col)] = QJsonValue::fromVariant(values.at(col));

    QJsonObject mainObj;
    mainObj["root"] = rootObj;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(QJsonDocument(mainObj).toJson(QJsonDocument::Indented));

    file.close();
    statusBar()->showMessage(tr("Экспортировано в %1").arg(filePath));
}

void MainWindow::exportCsv(int row)
{
    if (row < 0 || row >= m_model->rowCount()) {
        QMessageBox::information(this, tr("Информация"), tr("Выберите строку для экспорта."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, tr("Экспорт CSV"), QString(), tr("Файлы CSV (*.csv)"));
    if (filePath.isEmpty()) return;

    QStringList headers = getModelHeaders();
    QList<QVariant> values = getRowData(row);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << headers.join(",") << "\n";

    QStringList rowValues;
    for (int col = 0; col < values.size(); ++col) {
        QString val = values.at(col).toString();
        if (val.contains(',') || val.contains('"') || val.contains('\n'))
            val = "\"" + val.replace("\"", "\"\"") + "\"";
        rowValues.append(val);
    }
    out << rowValues.join(",") << "\n";

    file.close();
    statusBar()->showMessage(tr("Экспортировано в %1").arg(filePath));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_ui->tableView->viewport() && event->type() == QEvent::ContextMenu) {
        QContextMenuEvent *contextEvent = (QContextMenuEvent *)event;
        showContextMenu(contextEvent->pos());
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::deleteSelected()
{
    if (!m_dbReady)
        return;

    QModelIndex index = m_ui->tableView->currentIndex();
    if (!index.isValid())
        return;

    int row = index.row();
    int rowId = m_model->rowId(row);

    if (rowId < 0)
        return;

    m_worker->deleteRow(m_tableName, rowId);
    refreshData();
}

void MainWindow::onDataReady(const QList<QList<QVariant>> &rows, const QStringList &headers, const QList<int> &rowIds)
{
    m_model->setData(rows, headers, rowIds);
    statusBar()->showMessage(tr("Загружено строк: %1").arg(rows.size()));
}

void MainWindow::onOperationFinished(bool success, const QString &message)
{
    if (message.contains("БД открыта")) {
        m_dbReady = true;
        refreshData();
    } else if (message.contains("БД готова")) {
        if (!m_dbReady) m_dbReady = true;
        refreshData();
    } else if (!success) {
        statusBar()->showMessage(tr("Ошибка: %1").arg(message));
    }
}

void MainWindow::onImportReady(bool success, const QList<QList<QVariant>> &rows, const QStringList &headers, const QString &error)
{
    if (m_progressDialog) {
        m_progressDialog->hide();
    }

    if (!success || rows.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось импортировать файл: %1").arg(error));
        return;
    }

    m_worker->createTable(m_tableName, headers);

    for (const QList<QVariant> &row : rows) {
        m_worker->insertRow(m_tableName, row, headers);
    }

    statusBar()->showMessage(tr("Импортировано строк: %1").arg(rows.size()));
    m_dbReady = true;
    refreshData();
}
