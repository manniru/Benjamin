#ifndef CH_PROCESSOR_L_H
#define CH_PROCESSOR_L_H

#include <QObject>
#include <QThread>
#include "backend.h"
#include "config.h"
#include "ch_channel_l.h"

#define CH_BACKSPACE_CODE 16777219
#define CH_ESCAPE_CODE    16777216
#define CH_F1_CODE        16777264
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2

class ChProcessorL : public QObject
{
    Q_OBJECT
public:
    ChProcessorL(ChChannelL *ch, QObject *ui, QObject *parent = NULL);
    ~ChProcessorL();

public slots:
    void showUI(QString text);
    void key(int key);

private:
    void strToPos(QString input, int *x, int *y);
    void setPos(int x, int y);
    void setPosFine(int key);
    void hideUI();
    void createStatFile();
    void rmStatFile();

    QString   key_buf; //requested word
    QObject  *root;
    int       count_x; //read from qml
    int       count_y; //read from qml
    int       click_mode;
    int       meta_mode; // get a number at end
};

#endif // CH_PROCESSOR_L_H
