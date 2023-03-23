#ifndef AB_WIN_API_H
#define AB_WIN_API_H

#include <windows.h>
#include <QDebug>

QString getLinkPath(QString name);
QString getLinkPathA(QString name);
QString getLinkPathB(QString name);
QString findAppPath(QString path, QString pattern,
                    QString black_list);
HRESULT resolveIt(LPCSTR lnk_path, char *target);

#endif // AB_WIN_API_H
