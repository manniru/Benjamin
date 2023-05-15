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

#define CH_BACKSPACE_CODE 16777219
#define CH_ESCAPE_CODE    16777216
#define CH_F1_CODE        16777264
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2
#define CH_DOUBLE_CLICK 3
#define CH_PERSIST      4
#define CH_DRAG         5

class ChProcessorW : public QObject
{
    Q_OBJECT
public:
    ChProcessorW(QObject *ui, QObject *parent = NULL);
    ~ChProcessorW();

public slots:
    void showUI(QString text);
    void keyPressed(int key);

private:
    void processNatoKey(int key);
    void strToPos(QString input, int *x, int *y);
    void setPos(int x, int y);
    void setPosFine(int key);
    void hideUI();

    void setFocus();

    int        count_x; //read from qml
    int        count_y; //read from qml
    int        click_mode;
    int        meta_mode; // get a number at end
    int        drag_mode;
    QString    key_buf; //requested word
    ChExecW   *exec;
    QObject   *root;
};

#endif // CH_PROCESSOR_W_H
