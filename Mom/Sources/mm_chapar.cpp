#include "mm_chapar.h"
#include <QWindow>

MmChapar::MmChapar(QObject *root, QObject *parent) : QObject(parent)
{
    mon   = new MmMonitor(root);
    virt  = new MmVirt;
    state = new MmState();
    sound = new MmSound(state);
    bar   = new MmBar(root, virt, sound);
    key   = new MmKeyboard(virt, sound);

    mm_setKeyboard(key);

    QWindow *window = qobject_cast<QWindow *>(root);
    hWnd = (HWND)(window->winId());

    Register();
    SetSide();
}

MmChapar::~MmChapar()
{
    UnRegister();

    delete mon;
    delete key;
}

void MmChapar::UnRegister()
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    int success = !SHAppBarMessage(ABM_REMOVE, &abd);
    qDebug() << "UnRegister:" << success;

}

void MmChapar::Register()
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    int is_Registered = SHAppBarMessage(ABM_NEW, &abd);
    qDebug() << is_Registered;
}

//BOOL AppBar_SetSide(HWND hwnd)
BOOL MmChapar::SetSide()
{
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = BB_BAR_HEIGHT;

    // Fill out the APPBARDATA struct with the basic information
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    abd.rc = rc;
    abd.uEdge = ABE_TOP;

    // Tell the system we're moving to this new approved position.
    SHAppBarMessage(ABM_SETPOS, &abd);

    return TRUE;
}
