#ifndef CHANNEL_H
#define CHANNEL_H

#include "backend.h"
#include <QObject>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "QThread"
#include "config.h"

#define CH_BACKSPACE_CODE 16777219
#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

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
    void reset();

    QString   key_buf; //requested word
    QObject  *root;
    int       count_x; //read from qml
    int       count_y; //read from qml
};



#endif // CHANNEL_H
