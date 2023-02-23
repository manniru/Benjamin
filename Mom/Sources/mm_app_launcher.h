#ifndef MM_APP_LAUNCHER_H
#define MM_APP_LAUNCHER_H

#include <Windows.h>
#include <QtDebug>
#include "mm_api.h"
#include "mm_win32.h"

class MmAppLauncher : public QObject
{
    Q_OBJECT
public:
    explicit MmAppLauncher(QObject *parent = nullptr);
    ~MmAppLauncher();

    void focusOpen(QString shortcut);

private:
    MmApplication getApplication(QString shortcut_name, QString win_title);

};

#endif // MM_APP_LAUNCHER_H
