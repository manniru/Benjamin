#ifndef CHANNEL_H
#define CHANNEL_H

#include "backend.h"
#include <QObject>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "QThread"
#include "config.h"

#define CH_BACKSPACE_CODE 16777219
#define CH_ESCAPE_CODE    16777216
#define CH_F1_CODE        16777264
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

#define CH_LEFT_CLICK   0
#define CH_NO_CLICK     1
#define CH_RIGHT_CLICK  2

class Channel : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", COM_NAME)
public:
    Channel(QObject *ui, QObject *parent = NULL);
    ~Channel();

public slots:
    void showUI(const QString &text);
    void keyPressed(int key);

private:
    void ConnectDBus();
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



#endif // CHANNEL_H
