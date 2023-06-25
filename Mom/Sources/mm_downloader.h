#ifndef MM_DOWNLOADER_H
#define MM_DOWNLOADER_H

#include <QObject>
#include <QtNetwork>

class MmDownloader : public QObject
{
    Q_OBJECT
public:
    explicit MmDownloader(QObject *parent = nullptr);
    ~MmDownloader();

    void download(QString url);

private slots:
    void finished(QNetworkReply *reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QNetworkAccessManager *man;
    QNetworkReply         *reply;
};

#endif // MM_DOWNLOADER_H
