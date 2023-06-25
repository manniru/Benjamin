#include "mm_downloader.h"

MmDownloader::MmDownloader(QObject *parent) : QObject(parent)
{
    // Create an instance of QNetworkAccessManager
    man = new QNetworkAccessManager(this);

    // Connect the signal for finished downloads to a slot
    connect(man, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(finished(QNetworkReply *)));
}

MmDownloader::~MmDownloader()
{

}

void MmDownloader::download(QString url)
{
   QUrl main_url(url);

   // Start the download by sending a GET request
   QNetworkRequest request(main_url);
   reply = man->get(request);

   // Optionally, you can also track the download progress
   connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
           this, SLOT(downloadProgress(qint64, qint64)));

   qDebug() << "Errors:";
   qDebug() << QSslSocket::sslLibraryBuildVersionString();
   qDebug() << QSslSocket::supportsSsl();
   qDebug() << QSslSocket::sslLibraryVersionString();
   qDebug() << "Finish-E";
}

void MmDownloader::finished(QNetworkReply *reply)
{
    if( reply->error()==QNetworkReply::NoError )
    {
        // File download was successful
        QByteArray downloadedData = reply->readAll();
        qDebug() << "Download Data =" << downloadedData;
        // Process the downloaded data as needed
    }
    else
    {
        // File download encountered an error
        qDebug() << "Download error:" << reply->errorString();
    }

    // Clean up the network reply
    reply->deleteLater();
}

void MmDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << "Download progress:" << bytesReceived << "/" << bytesTotal;
}



