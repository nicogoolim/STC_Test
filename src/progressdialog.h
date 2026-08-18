#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

class QProgressBar;
class QLabel;
class QListWidget;
class QPushButton;

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

    void setProgress(int current, int total, const QString &currentFile);
    void setResults(int successCount, int errorCount, const QStringList &errorFiles);
    void reset();

private slots:
    void onCloseClicked();

private:
    void setupUi();

    QProgressBar *m_progressBar;
    QLabel *m_currentFileLabel;
    QLabel *m_successLabel;
    QLabel *m_errorLabel;
    QListWidget *m_errorListWidget;
    QPushButton *m_closeButton;
    int m_totalFiles;
};

#endif // PROGRESSDIALOG_H
