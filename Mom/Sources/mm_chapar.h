#ifndef MM_CHAPAR_H
#define MM_CHAPAR_H

#include <QObject>
#include "mm_bar.h"

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

class MmChapar : public QObject
{
    Q_OBJECT
public:
    explicit MmChapar(QObject *root, QObject *parent = nullptr);
    ~MmChapar();

private:
    void Register();
    void UnRegister();
    BOOL SetSide();

    MmBar *bar;
    HWND hWnd;
};

#endif // MM_CHAPAR_H
