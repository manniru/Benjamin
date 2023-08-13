#ifndef CH_SCREENSHOT_H
#define CH_SCREENSHOT_H

#include <QObject>
#include <QString>
#include <QVector>

#ifdef WIN32
#include <Windows.h>
#endif

class ChScreenshot : public QObject
{
    Q_OBJECT
public:
    explicit ChScreenshot(QObject *parent = nullptr);
    ~ChScreenshot();

    void takeShot(int shot_x, int shot_y,
                  int shot_w, int shot_h);

    void sendToClipboard();

private:
#ifdef WIN32
    HDC screenDC;
    HDC memDC;
    HBITMAP bitmap;
    HBITMAP oldBitmap;
#endif
};

#endif // CH_SCREENSHOT_H
