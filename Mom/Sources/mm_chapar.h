#ifndef MM_CHAPAR_H
#define MM_CHAPAR_H

#include <QObject>
#include "mm_bar.h"
#include "mm_monitor.h"
#include "mm_keyboard.h"

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include <QScreen>

class MmChapar : public QObject
{
    Q_OBJECT
public:
    explicit MmChapar(QObject *root, QObject *parent = nullptr);
    ~MmChapar();

    MmKeyboard *key;

private:
    void UnRegister(HWND hWnd);
    BOOL barRegister(HWND hWnd, QRect screen);
    void barPlacement();
    void placeWindow(HWND hwnd, QRect screen);

    QVector<QWindow *> windows;
    MmMonitor  *mon;
    MmState    *state;
    MmVirt     *virt;
    MmBar      *bar;
    MmSound    *sound;
    QObject    *ui;
    QVector<HWND> hwnds;
};

#endif // MM_CHAPAR_H
