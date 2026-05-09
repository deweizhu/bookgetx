//
// Created by DeweiZhu on 2026/5/9.
//

#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>
#include <QtCore/QSet>

class QComboBox;
class QLineEdit;
class QMessageBox;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QWebEngineView;
class QNetworkAccessManager;
class QNetworkReply;

#include "HashStore.h"

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    enum class PlaceholderType { Number, Letter };

    void buildUi();
    void connectSignals();

    bool parseTemplateAndRange(const QString& inputUrl, QString& outTemplate, QString& errorMsg);
    QString buildPlaceholderValue(int value) const;
    QString buildUrl(const QString& templateUrl, int value) const;
    QString buildFilePath(int pageIndex) const;

    void setRunningUi(bool running);
    void logLine(const QString& line);

    void startDownload();
    void stopDownload();
    void finishTask();

    void prepareDownloadedHashes(const QString& outDir);
    void clearDownloadedRecords();

    void downloadCurrentPageWithRetry(int attempt);

    void initializeSaveDirectory();
    void browseSaveDirectory();

private:
    QPlainTextEdit* urlEdit_ = nullptr;
    QLineEdit* saveDirEdit_ = nullptr;
    QLineEdit* prefixEdit_ = nullptr;
    QComboBox* extBox_ = nullptr;
    QSpinBox* retrySpin_ = nullptr;
    QSpinBox* retryDelaySpin_ = nullptr;

    QSpinBox* placeholderLengthSpin_ = nullptr;
    QLineEdit* placeholderRangeEdit_ = nullptr;
    QComboBox* placeholderTypeBox_ = nullptr;

    QPushButton* startBtn_ = nullptr;
    QPushButton* stopBtn_ = nullptr;
    QPushButton* browseBtn_ = nullptr;
    QPushButton* clearRecordsBtn_ = nullptr;

    QProgressBar* progress_ = nullptr;
    QPlainTextEdit* log_ = nullptr;
    QWebEngineView* webView_ = nullptr;

    QNetworkAccessManager* manager_ = nullptr;

    QString templateUrl_;
    int startPage_ = 0;
    int endPage_ = 0;
    int currentPage_ = 0;
    int width_ = 4;
    bool stopRequested_ = false;
    int maxRetries_ = 3;
    int retryDelaySeconds_ = 3;

    PlaceholderType placeholderType_ = PlaceholderType::Number;
    int placeholderStart_ = 1;
    int placeholderEnd_ = 1000;
    int placeholderLength_ = 1;

    QSet<QString> downloadedHashes_;
    QFile errorLogFile_;
    HashStore hashStore_;
};
