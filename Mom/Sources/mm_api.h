#ifndef MM_API_H
#define MM_API_H

#include <QString>
#include <Windows.h>

QString mm_getLinkPath(QString path);
QString mm_getLinkPathA(QString path);
QString mm_getLinkPathB(QString path);
void mm_launchApp(QString app_name, QString arg="");
void mm_launchScript(QString path, QString arg="");
HRESULT mm_ResolveIt(LPCSTR lnk_path, char *target);

#endif // MM_API_H
