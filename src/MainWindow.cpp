//
// Created by DeweiZhu on 2026/5/9.
//

#include "MainWindow.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtWebEngineCore/QWebEngineCookieStore>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

MainWindow::MainWindow()
{
    setWindowTitle("BookgetX - Image Downloader");
    resize(1180, 820);

    buildUi();
    manager_ = new QNetworkAccessManager(this);

    QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
    QNetworkCookieJar* cookieJar = manager_->cookieJar();

    connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded, this,
            [cookieJar](const QNetworkCookie& cookie) { cookieJar->insertCookie(cookie); });

    connect(profile->cookieStore(), &QWebEngineCookieStore::cookieRemoved, this,
            [cookieJar](const QNetworkCookie& cookie) { cookieJar->deleteCookie(cookie); });

    connectSignals();
}

void MainWindow::buildUi()
{
    auto* inputGroup = new QGroupBox("下载参数");
    auto* inputLayout = new QGridLayout(inputGroup);
    inputLayout->setContentsMargins(12, 12, 12, 12);
    inputLayout->setHorizontalSpacing(10);
    inputLayout->setVerticalSpacing(8);
    inputLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto* urlLabel = new QLabel("下载URLs:");
    urlLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    urlEdit_ = new QPlainTextEdit();
    urlEdit_->setPlaceholderText(
        "请输入URL模板（必须包含占位符*），例如：\n"
        "https://babel.hathitrust.org/cgi/imgsrv/image?...&seq=*");
    urlEdit_->setPlainText("https://babel.hathitrust.org/cgi/imgsrv/"
                           "image?id=uc1.$b470134&attachment=0&tracker=D1&"
                           "format=image%2Fjpeg&size=full&seq=*");
    urlEdit_->setMinimumHeight(20);
    urlEdit_->setMaximumHeight(60);

    auto* placeholderLabel = new QLabel("占位符设置:");
    auto* placeholderLengthLabel = new QLabel("长度:");
    auto* placeholderRangeLabel = new QLabel("范围:");
    auto* placeholderTypeLabel = new QLabel("类型:");

    placeholderLengthSpin_ = new QSpinBox();
    placeholderLengthSpin_->setRange(1, 10);
    placeholderLengthSpin_->setValue(1);

    placeholderRangeEdit_ = new QLineEdit("1-1000");
    placeholderRangeEdit_->setPlaceholderText("例如: 1-1000 或 a-z");

    placeholderTypeBox_ = new QComboBox();
    placeholderTypeBox_->addItems({"数字", "字母"});

    auto* dirLabel = new QLabel("保存目录:");
    saveDirEdit_ = new QLineEdit(QDir::current().filePath("downloads"));
    browseBtn_ = new QPushButton("选择目录");

    auto* prefixLabel = new QLabel("文件名前缀:");
    prefixEdit_ = new QLineEdit();
    prefixEdit_->setPlaceholderText("例如：book_（可留空）");

    auto* extLabel = new QLabel("扩展名:");
    extBox_ = new QComboBox();
    extBox_->addItems({"jpg", "png", "jp2", "jpeg", "webp"});

    auto* retryLabel = new QLabel("失败重试:");
    retrySpin_ = new QSpinBox();
    retrySpin_->setRange(0, 20);
    retrySpin_->setValue(3);
    retrySpin_->setSuffix(" 次");

    auto* retryDelayLabel = new QLabel("重试间隔:");
    retryDelaySpin_ = new QSpinBox();
    retryDelaySpin_->setRange(1, 60);
    retryDelaySpin_->setValue(3);
    retryDelaySpin_->setSuffix(" 秒");

    startBtn_ = new QPushButton("开始下载");
    stopBtn_ = new QPushButton("停止");
    stopBtn_->setEnabled(false);
    clearRecordsBtn_ = new QPushButton("清空下载记录");

    inputLayout->addWidget(urlLabel, 0, 0, Qt::AlignLeft);
    inputLayout->addWidget(urlEdit_, 0, 1, 2, 6);

    inputLayout->addWidget(placeholderLabel, 2, 0, Qt::AlignLeft);
    inputLayout->addWidget(placeholderLengthLabel, 2, 1, Qt::AlignLeft);
    inputLayout->addWidget(placeholderLengthSpin_, 2, 2);
    inputLayout->addWidget(placeholderRangeLabel, 2, 3, Qt::AlignLeft);
    inputLayout->addWidget(placeholderRangeEdit_, 2, 4);
    inputLayout->addWidget(placeholderTypeLabel, 2, 5, Qt::AlignLeft);
    inputLayout->addWidget(placeholderTypeBox_, 2, 6);

    auto* placeholderInfo =
        new QLabel("占位符 * 值范围：数字 1-1000 或字母 a-z；长度 1-10（数字时补零）");
    inputLayout->addWidget(placeholderInfo, 3, 0, 1, 7);

    inputLayout->addWidget(dirLabel, 4, 0, Qt::AlignLeft);
    inputLayout->addWidget(saveDirEdit_, 4, 1, 1, 5);
    inputLayout->addWidget(browseBtn_, 4, 6);

    inputLayout->addWidget(prefixLabel, 5, 0, Qt::AlignLeft);
    inputLayout->addWidget(prefixEdit_, 5, 1);
    inputLayout->addWidget(extLabel, 5, 2, Qt::AlignLeft);
    inputLayout->addWidget(extBox_, 5, 3);
    inputLayout->addWidget(retryLabel, 5, 4, Qt::AlignLeft);
    inputLayout->addWidget(retrySpin_, 5, 5);

    inputLayout->addWidget(retryDelayLabel, 6, 0, Qt::AlignLeft);
    inputLayout->addWidget(retryDelaySpin_, 6, 1);

    auto* btnRow = new QHBoxLayout();
    btnRow->setAlignment(Qt::AlignLeft);
    btnRow->addWidget(startBtn_);
    btnRow->addWidget(stopBtn_);
    btnRow->addWidget(clearRecordsBtn_);
    inputLayout->addLayout(btnRow, 6, 2, 1, 5, Qt::AlignLeft);

    auto* statusGroup = new QGroupBox("进度与日志");
    auto* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(12, 12, 12, 12);
    statusLayout->setSpacing(8);

    progress_ = new QProgressBar();
    progress_->setTextVisible(true);

    log_ = new QPlainTextEdit();
    log_->setReadOnly(true);
    log_->setMinimumHeight(140);

    statusLayout->addWidget(progress_);
    statusLayout->addWidget(log_);

    auto* previewGroup = new QGroupBox("当前页面预览（用于人工验证）");
    auto* previewLayout = new QVBoxLayout(previewGroup);
    previewLayout->setContentsMargins(12, 12, 12, 12);
    webView_ = new QWebEngineView();
    previewLayout->addWidget(webView_);

    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);
    leftLayout->addWidget(inputGroup, 0);
    leftLayout->addWidget(statusGroup, 1);

    auto* rootSplitter = new QSplitter(Qt::Horizontal, this);
    rootSplitter->addWidget(leftPanel);
    rootSplitter->addWidget(previewGroup);
    rootSplitter->setChildrenCollapsible(false);
    rootSplitter->setStretchFactor(0, 1);
    rootSplitter->setStretchFactor(1, 1);
    rootSplitter->setSizes({500, 500});

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(rootSplitter);
}

void MainWindow::connectSignals()
{
    connect(browseBtn_, &QPushButton::clicked, this, [this]()
    {
        const QString dir = QFileDialog::getExistingDirectory(this, "选择保存目录", saveDirEdit_->text());
        if (!dir.isEmpty())
        {
            saveDirEdit_->setText(dir);
        }
    });

    connect(startBtn_, &QPushButton::clicked, this, &MainWindow::startDownload);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::stopDownload);
    connect(clearRecordsBtn_, &QPushButton::clicked, this, &MainWindow::clearDownloadedRecords);
}

bool MainWindow::parseTemplateAndRange(const QString& inputUrl, QString& outTemplate, QString& errorMsg)
{
    const QString trimmedUrl = inputUrl.trimmed();
    if (trimmedUrl.isEmpty())
    {
        errorMsg = "URL模板不能为空";
        return false;
    }
    if (!trimmedUrl.contains('*'))
    {
        errorMsg = "URL模板必须包含占位符 *";
        return false;
    }

    placeholderLength_ = placeholderLengthSpin_->value();
    const QString rangeText = placeholderRangeEdit_->text().trimmed();
    const QString typeText = placeholderTypeBox_->currentText().trimmed();

    if (typeText == "数字")
    {
        placeholderType_ = PlaceholderType::Number;
        static const QRegularExpression numRe(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");
        const QRegularExpressionMatch m = numRe.match(rangeText);
        if (!m.hasMatch())
        {
            errorMsg = "数字范围格式错误，应为：1-1000";
            return false;
        }

        placeholderStart_ = m.captured(1).toInt();
        placeholderEnd_ = m.captured(2).toInt();
        if (placeholderStart_ < 0 || placeholderEnd_ < placeholderStart_)
        {
            errorMsg = "数字范围无效";
            return false;
        }
    }
    else
    {
        placeholderType_ = PlaceholderType::Letter;
        static const QRegularExpression letterRe(R"(^\s*([A-Za-z])\s*-\s*([A-Za-z])\s*$)");
        const QRegularExpressionMatch m = letterRe.match(rangeText);
        if (!m.hasMatch())
        {
            errorMsg = "字母范围格式错误，应为：a-z";
            return false;
        }

        const QChar c1 = m.captured(1).at(0).toLower();
        const QChar c2 = m.captured(2).at(0).toLower();
        placeholderStart_ = c1.unicode();
        placeholderEnd_ = c2.unicode();
        if (placeholderEnd_ < placeholderStart_)
        {
            errorMsg = "字母范围无效";
            return false;
        }
    }

    outTemplate = trimmedUrl;
    outTemplate.replace('*', "%PAGE%");
    return true;
}

QString MainWindow::buildPlaceholderValue(int value) const
{
    if (placeholderType_ == PlaceholderType::Number)
    {
        return QString("%1").arg(value, placeholderLength_, 10, QLatin1Char('0'));
    }
    return QString(QChar(static_cast<char16_t>(value)));
}

QString MainWindow::buildUrl(const QString& templateUrl, int value) const
{
    QString out = templateUrl;
    out.replace("%PAGE%", buildPlaceholderValue(value));
    return out;
}

QString MainWindow::buildFilePath(int pageIndex) const
{
    const QString ext = extBox_->currentText().trimmed();
    const QString prefix = prefixEdit_->text().trimmed();
    const QString number = QString("%1").arg(pageIndex, width_, 10, QLatin1Char('0'));
    const QString fileName = QString("%1%2.%3").arg(prefix, number, ext);
    return QDir(saveDirEdit_->text().trimmed()).filePath(fileName);
}

void MainWindow::setRunningUi(bool running)
{
    startBtn_->setEnabled(!running);
    stopBtn_->setEnabled(running);

    urlEdit_->setEnabled(!running);
    saveDirEdit_->setEnabled(!running);
    browseBtn_->setEnabled(!running);
    prefixEdit_->setEnabled(!running);
    extBox_->setEnabled(!running);
    retrySpin_->setEnabled(!running);
    retryDelaySpin_->setEnabled(!running);
    placeholderLengthSpin_->setEnabled(!running);
    placeholderRangeEdit_->setEnabled(!running);
    placeholderTypeBox_->setEnabled(!running);
}

void MainWindow::logLine(const QString& line)
{
    log_->appendPlainText(line);
}

void MainWindow::prepareDownloadedHashes(const QString& outDir)
{
    downloadedHashes_.clear();

    const QSet<QString> fromDisk = hashStore_.load();
    for (const QString& h : fromDisk)
    {
        downloadedHashes_.insert(h);
    }

    Q_UNUSED(outDir)
}

void MainWindow::startDownload()
{
    QString parsedTemplate;
    QString err;
    if (!parseTemplateAndRange(urlEdit_->toPlainText(), parsedTemplate, err))
    {
        QMessageBox::warning(this, "输入错误", err);
        return;
    }

    const QString outDir = saveDirEdit_->text().trimmed();
    if (outDir.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "保存目录不能为空");
        return;
    }

    QDir dir(outDir);
    if (!dir.exists() && !dir.mkpath("."))
    {
        QMessageBox::warning(this, "目录错误", "无法创建保存目录");
        return;
    }

    errorLogFile_.setFileName(dir.filePath("error_log.txt"));
    if (!errorLogFile_.open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::warning(this, "文件错误", "无法打开错误日志文件");
        return;
    }

    prepareDownloadedHashes(outDir);

    stopRequested_ = false;
    templateUrl_ = parsedTemplate;
    startPage_ = placeholderStart_;
    endPage_ = placeholderEnd_;
    currentPage_ = startPage_;

    width_ = qMax(4, QString::number(endPage_ - startPage_ + 1).size());
    maxRetries_ = retrySpin_->value();
    retryDelaySeconds_ = retryDelaySpin_->value();

    progress_->setRange(startPage_, endPage_);
    progress_->setValue(startPage_ - 1);
    log_->clear();

    logLine(QString("开始下载范围: %1 -> %2").arg(startPage_).arg(endPage_));
    logLine(QString("URL模板: %1").arg(templateUrl_));
    logLine(QString("重试策略: 失败后最多重试 %1 次").arg(maxRetries_));
    logLine(QString("重试间隔: %1 秒").arg(retryDelaySeconds_));

    setRunningUi(true);
    downloadCurrentPageWithRetry(0);
}

void MainWindow::stopDownload()
{
    stopRequested_ = true;
    logLine("已请求停止");
}

void MainWindow::finishTask()
{
    logLine("任务结束");
    setRunningUi(false);
    if (errorLogFile_.isOpen())
    {
        errorLogFile_.close();
    }
}

void MainWindow::clearDownloadedRecords()
{
    const auto ret = QMessageBox::question(
        this,
        "确认清空",
        "确定要清空已下载URL记录吗？\n此操作不会删除已下载图片文件。",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret != QMessageBox::Yes)
    {
        return;
    }

    downloadedHashes_.clear();
    if (!hashStore_.clear())
    {
        logLine("警告: 清空下载记录文件失败");
    }
    else
    {
        logLine("已清空下载记录");
    }
}

void MainWindow::downloadCurrentPageWithRetry(int attempt)
{
    if (stopRequested_ || currentPage_ > endPage_)
    {
        finishTask();
        return;
    }

    const int value = currentPage_;
    const int fileIndex = value - startPage_ + 1;
    const QString pageUrl = buildUrl(templateUrl_, value);
    const QString urlHash = QString::fromLatin1(
        QCryptographicHash::hash(pageUrl.toUtf8(), QCryptographicHash::Md5).toHex());

    if (downloadedHashes_.contains(urlHash))
    {
        logLine(QString("跳过已下载: %1").arg(pageUrl));
        ++currentPage_;
        QTimer::singleShot(700, this, [this]() { downloadCurrentPageWithRetry(0); });
        return;
    }

    if (attempt == 0)
    {
        webView_->load(QUrl(pageUrl));
    }

    QNetworkRequest req{QUrl(pageUrl)};
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/148.0.0.0 Safari/537.36");

    QNetworkReply* reply = manager_->get(req);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, value, fileIndex, attempt, pageUrl, urlHash]()
            {
                if (reply->error() == QNetworkReply::NoError)
                {
                    const QString path = buildFilePath(fileIndex);
                    QFile f(path);
                    if (f.open(QIODevice::WriteOnly))
                    {
                        f.write(reply->readAll());
                        f.close();
                        downloadedHashes_.insert(urlHash);
                        if (!hashStore_.append(urlHash))
                        {
                            logLine("警告: 无法写入下载记录文件");
                        }
                        logLine(QString("已保存: %1").arg(QFileInfo(path).fileName()));
                    }
                    else
                    {
                        logLine(QString("写入失败: %1").arg(path));
                    }

                    reply->deleteLater();
                    progress_->setValue(value);
                    ++currentPage_;
                    QTimer::singleShot(700, this, [this]() { downloadCurrentPageWithRetry(0); });
                    return;
                }

                const QString err = reply->errorString();
                reply->deleteLater();

                if (attempt < maxRetries_)
                {
                    const int nextAttempt = attempt + 1;
                    logLine(QString("下载失败: %1，%2 秒后重试 %3/%4")
                                .arg(pageUrl)
                                .arg(retryDelaySeconds_)
                                .arg(nextAttempt)
                                .arg(maxRetries_));
                    QTimer::singleShot(retryDelaySeconds_ * 1000, this, [this, nextAttempt]()
                    {
                        downloadCurrentPageWithRetry(nextAttempt);
                    });
                }
                else
                {
                    logLine(QString("下载失败(已跳过): %1, 错误: %2").arg(pageUrl).arg(err));
                    progress_->setValue(value);
                    ++currentPage_;

                    if (errorLogFile_.isOpen())
                    {
                        QTextStream out(&errorLogFile_);
                        out << pageUrl << "\n";
                    }

                    QTimer::singleShot(700, this, [this]() { downloadCurrentPageWithRetry(0); });
                }
            });
}