#include "mm_chapar.h"
#include <QQuickWindow>
#include <QGuiApplication>

MmChapar::MmChapar(QObject *root, QObject *parent):
    QObject(parent)
{
    ui = root;
    mon   = new MmMonitor(ui);
    virt  = new MmVirt;
    state = new MmState();
    sound = new MmSound(state);
    key   = new MmKeyboard(virt, sound);

    mm_setKeyboard(key);
    barPlacement();

    bar = new MmBar(windows, virt, sound);
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
    QList<QScreen *> screens = QGuiApplication::screens();
    int len = screens.size();
    windows.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        int width = screens[i]->geometry().width();
        int x = screens[i]->geometry().x();
        int y = screens[i]->geometry().y();

        QString object_name = "bar" + QString::number(i+1);
        QWindow *window = ui->findChild<QWindow *>(object_name);
        QQmlProperty::write(window, "width", width);
        QQmlProperty::write(window, "x", x);
        QQmlProperty::write(window, "y", y);
        window->show();
        windows[i] = window;

        HWND hWnd = (HWND)(window->winId());
        hwnds.push_back(hWnd);
        barRegister(hWnd, screens[i]->geometry());
    }
}

BOOL MmChapar::barRegister(HWND hWnd, QRect screen)
{
    // create new bar
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    int is_Registered = SHAppBarMessage(ABM_NEW, &abd);
    qDebug() << "reg" << is_Registered;

    // set app bar position
    RECT rc;
    rc.top = screen.y();
    rc.left = screen.x();
    rc.right = rc.left + screen.width();
    rc.bottom = rc.top + MM_BAR_HEIGHT;
//    qDebug() << "coord>>" << rc.top << rc.bottom << rc.left << rc.right;

    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    abd.rc = rc;
    abd.uEdge = ABE_TOP;

    // Tell the system we're moving to this new approved position.
    SHAppBarMessage(ABM_SETPOS, &abd);

    return TRUE;
}
