#ifndef CH_EXEC_W_H
#define CH_EXEC_W_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "backend.h"
#include "config.h"
#include "ch_monitor.h"

class ChExecW : public QObject
{
    Q_OBJECT
public:
    ChExecW(QObject *ui, QObject *parent = NULL);
    ~ChExecW();

    void setLanguage();
    void activateWindow();
    void bringToFront();
    void updateScreen(QString cmd);

    void sendMouseKey(int val);
    void mousePress(int btn);
    void mouseRelease(int btn);
signals:

public slots:

private:
    int mid_x;
    int mid_y;
    QObject  *root;
    HWND      hWnd;
    ChMonitor *mon;
};

#endif // CH_EXEC_W_H
