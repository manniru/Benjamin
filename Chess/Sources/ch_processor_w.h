#ifndef CH_PROCESSOR_W_H
#define CH_PROCESSOR_W_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "backend.h"
#include "config.h"
#include "ch_channel_w.h"
#include "ch_keyboard_w.h"
#include "ch_exec_w.h"
#include "ch_screenshot.h"

#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2
#define CH_DOUBLE_CLICK 3
#define CH_PERSIST      4
#define CH_DRAG         5
#define CH_SHOT_I       6
#define CH_SHOT_II      7

class ChProcessorW : public QObject
{
    Q_OBJECT
public:
    ChProcessorW(QObject *ui, QObject *parent = NULL);
    ~ChProcessorW();

public slots:
    void showUI(QString text);
    void keyReceived(int val);
    void meta();
    void cancel();

private:
    void processNatoKey(int key);
    void strToPos(QString input, int *x, int *y);
    void setPos(int x, int y);
    void setPosFine(int key);
    void hideUI();

    void setFocus();
    void setCursor(int x, int y);

    int count_x; //read from qml
    int count_y; //read from qml
    int click_mode;
    int meta_mode; // get a number at end
    int drag_mode;

    int shot_x;
    int shot_y;
    int shot_w;
    int shot_h;

    QString       key_buf; //requested word
    ChExecW      *exec;
    ChScreenshot *shot;
    QObject   *root;
};

#endif // CH_PROCESSOR_W_H
