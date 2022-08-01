#ifndef BB_CHAPAR_H
#define BB_CHAPAR_H

#include <QObject>
#include "bb_bar.h"

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

//#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

class BbChapar : public QObject
{
    Q_OBJECT
public:
    explicit BbChapar(QObject *root, QObject *parent = nullptr);
    ~BbChapar();

private:

    void Register();
    void UnRegister();
    BOOL SetSide();
    BbBar *bar;
    HWND hWnd;
};

#endif // BB_CHAPAR_H
