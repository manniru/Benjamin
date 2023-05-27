#ifndef CH_SCREENSHOT_H
#define CH_SCREENSHOT_H

#include <QObject>
#include <QString>
#include <QVector>

#include <Windows.h>

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
    HDC screenDC;
    HDC memDC;
    HBITMAP bitmap;
    HBITMAP oldBitmap;
};

#endif // CH_SCREENSHOT_H
