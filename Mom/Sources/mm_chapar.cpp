#include "mm_chapar.h"
#include <QWindow>
#include <QGuiApplication>
#include <QScreen>

MmChapar::MmChapar(QObject *root, QObject *parent):
    QObject(parent)
{
    ui = root;
    mon   = new MmMonitor(ui);
    virt  = new MmVirt;
    state = new MmState();
    sound = new MmSound(state);
    bar   = new MmBar(ui, virt, sound);
    key   = new MmKeyboard(virt, sound);

    mm_setKeyboard(key);
    barPlacement();
}

MmChapar::~MmChapar()
{
    int len = hwnds.size();
    for( int i=0 ; i<len ; i++ )
    {
        UnRegister(hwnds[i]);
    }
    hwnds.clear();

    delete mon;
    delete key;
}

void MmChapar::UnRegister(HWND hWnd)
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    int success = !SHAppBarMessage(ABM_REMOVE, &abd);
    qDebug() << "UnRegister:" << success;

}

void MmChapar::barPlacement()
{
    // Set screen width
    QScreen *screen = QGuiApplication::primaryScreen();
    int width = screen->geometry().width();

    int len = 1;
    for( int i=0 ; i<len ; i++ )
    {
        QString object_name = "bar" + QString::number(i+1);
        QWindow *window = ui->findChild<QWindow *>(object_name);
        qDebug() << object_name << width;
        QQmlProperty::write(window, "width", width);

        HWND hWnd = (HWND)(window->winId());
        hwnds.push_back(hWnd);
        barRegister(hWnd);
    }
}

//BOOL AppBar_SetSide(HWND hwnd)
BOOL MmChapar::barRegister(HWND hWnd)
{
    // create new bar
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    int is_Registered = SHAppBarMessage(ABM_NEW, &abd);
    qDebug() << is_Registered;

    // set app bar position
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = BB_BAR_HEIGHT;

    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    abd.rc = rc;
    abd.uEdge = ABE_TOP;

    // Tell the system we're moving to this new approved position.
    SHAppBarMessage(ABM_SETPOS, &abd);

    return TRUE;
}
