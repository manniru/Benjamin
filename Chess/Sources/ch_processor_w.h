#ifndef CH_PROCESSOR_W_H
#define CH_PROCESSOR_W_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "backend.h"
#include "config.h"
#include "ch_channel_w.h"
#include "ch_keyboard_w.h"

#define CH_BACKSPACE_CODE 16777219
#define CH_ESCAPE_CODE    16777216
#define CH_F1_CODE        16777264
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2
#define CH_PERSIST      3

class ChProcessorW : public QObject
{
    Q_OBJECT
public:
    ChProcessorW(ChChannelW *ch, QObject *ui, QObject *parent = NULL);
    ~ChProcessorW();

public slots:
    void showUI(QString text);
    void keyPressed(int key);

private:
    void strToPos(QString input, int *x, int *y);
    void setPos(int x, int y);
    void setPosFine(int key);
    void hideUI();
    void createStatFile();
    void rmStatFile();
    void activateWindow();

    void sendLeftKey();
    void sendRightKey();
    void sendMiddleKey();

    void setFocus();
    void sendKey(int key_val);
    void pressKey(int key_val);
    void releaseKey(int key_val);

    QString   key_buf; //requested word
    QObject  *root;
    int       count_x; //read from qml
    int       count_y; //read from qml
    int       click_mode;
    int       meta_mode; // get a number at end
    HWND      hWnd;
};

#endif // CH_PROCESSOR_W_H
