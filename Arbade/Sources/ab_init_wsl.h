#ifndef AB_INIT_WSL_H
#define AB_INIT_WSL_H

#include <QObject>
#include <QtNetwork>
#include "ab_win_api.h"
#include "backend.h"
#include "config.h"

#define AB_IMAGE_URL "https://github.com/bijanbina/KalB/releases/download/WSL/KalB.rar"

class AbInitWSL : public QObject
{
    Q_OBJECT
public:
    explicit AbInitWSL(QObject *parent = nullptr);
    ~AbInitWSL();

    QString getWslPath();

public slots:
    void createWSL(QString drive);

private slots:

private:
    void downloadImage(QString path);
    void uncompImage(QString path); // uncompress image
    int checkWSLInstalled();

    QNetworkAccessManager net_man;
    QNetworkReply* reply;
    QFile rar_file;
};

#endif // AB_INIT_WSL_H
