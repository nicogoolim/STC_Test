#include "progressdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <QApplication>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
{
    m_progressBar = 0;
    m_currentFileLabel = 0;
    m_successLabel = 0;
    m_errorLabel = 0;
    m_errorListWidget = 0;
    m_closeButton = 0;
    m_totalFiles = 0;
    setupUi();
    setWindowTitle(tr("Прогресс импорта"));
    resize(500, 400);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);

    m_currentFileLabel = new QLabel(tr("Подготовка..."), this);
    mainLayout->addWidget(m_currentFileLabel);
    m_successLabel = new QLabel(tr("Успешно импортировано: 0"), this);
    mainLayout->addWidget(m_successLabel);
    m_errorLabel = new QLabel(tr("Ошибки: 0"), this);
    mainLayout->addWidget(m_errorLabel);

    QLabel *errorListLabel = new QLabel(tr("Файлы с ошибками:"), this);
    mainLayout->addWidget(errorListLabel);
    m_errorListWidget = new QListWidget(this);
    m_errorListWidget->setMaximumHeight(150);
    mainLayout->addWidget(m_errorListWidget);

    m_closeButton = new QPushButton(tr("Закрыть"), this);
    m_closeButton->setEnabled(false);
    connect(m_closeButton, &QPushButton::clicked, this, &ProgressDialog::onCloseClicked);
    mainLayout->addWidget(m_closeButton);
}

void ProgressDialog::setProgress(int current, int total, const QString &currentFile)
{
    m_totalFiles = total;
    m_progressBar->setRange(0, total);
    m_progressBar->setValue(current);
    m_currentFileLabel->setText(tr("Обработка: %1 (%2 из %3)").arg(currentFile).arg(current).arg(total));
    qApp->processEvents();
}

void ProgressDialog::setResults(int successCount, int errorCount, const QStringList &errorFiles)
{
    m_successLabel->setText(tr("Успешно импортировано: %1").arg(successCount));
    m_errorLabel->setText(tr("Ошибки: %1").arg(errorCount));
    m_errorListWidget->clear();
    for (int i = 0; i < errorFiles.size(); ++i)
        m_errorListWidget->addItem(errorFiles.at(i));
    m_progressBar->setRange(0, m_totalFiles);
    m_progressBar->setValue(m_totalFiles);
    m_currentFileLabel->setText(tr("Импорт завершен"));
    m_closeButton->setEnabled(true);
}

void ProgressDialog::reset()
{
    m_progressBar->setRange(0, 0);
    m_progressBar->setValue(0);
    m_currentFileLabel->setText(tr("Подготовка..."));
    m_successLabel->setText(tr("Успешно импортировано: 0"));
    m_errorLabel->setText(tr("Ошибки: 0"));
    m_errorListWidget->clear();
    m_closeButton->setEnabled(false);
    m_totalFiles = 0;
}

void ProgressDialog::onCloseClicked()
{
    accept();
}
