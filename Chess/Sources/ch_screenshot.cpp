#include "ch_screenshot.h"
#include <QWindow>
#include <QDebug>

ChScreenshot::ChScreenshot(QObject *parent) : QObject(parent)
{
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)this);
}

ChScreenshot::~ChScreenshot()
{
}

void ChScreenshot::takeShot(int shot_x, int shot_y,
                            int shot_w, int shot_h)
{
    // Get screen device context
    screenDC = GetDC(NULL);
    if( screenDC==NULL )
        return;

    // Create a compatible device context for the screenshot
    memDC = CreateCompatibleDC(screenDC);
    if( memDC==NULL )
    {
        ReleaseDC(NULL, screenDC);
        return;
    }

    // Create a compatible bitmap for the screenshot
    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, shot_w, shot_h);
    if( bitmap==NULL )
    {
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return;
    }

    // Select the bitmap into the memory device context
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);

    // Copy the screen contents to the memory device context
    BitBlt(memDC, 0, 0, shot_w, shot_h,
           screenDC, shot_x, shot_y, SRCCOPY);

    sendToClipboard();

    // Cleanup
    SelectObject(memDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);

    return;
}

void ChScreenshot::sendToClipboard()
{
    OpenClipboard(NULL);
    EmptyClipboard();

    // Set the clipboard data format to a bitmap
    SetClipboardData(CF_BITMAP, bitmap);

    // Close the clipboard
    CloseClipboard();
}
