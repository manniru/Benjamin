#ifndef AB_INIT_WSL_H
#define AB_INIT_WSL_H

#include <QObject>
#include <QDir>
#include <QtNetwork>
#include <windows.h>
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

QString getLinkPath(QString name);
QString getLinkPathA(QString name);
QString getLinkPathB(QString name);
QString findAppPath(QString path, QString pattern,
                    QString black_list);
HRESULT resolveIt(LPCSTR lnk_path, char *target);

#endif // AB_INIT_WSL_H
