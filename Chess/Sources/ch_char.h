#ifndef CH_CHAR_H
#define CH_CHAR_H

#include <QObject>
#include <windows.h> // for key codes
#include "backend.h"
#include "config.h"

#define CH_KEY_MIN        ('0'-1)
#define CH_KEY_MAX        ('Z'+1)

class ChChar : public QObject
{
    Q_OBJECT
public:
    ChChar(QObject *ui, QObject *parent = NULL);
    ~ChChar();

    void strToPos(QString input, int *x, int *y);
private:
    void addCell(QString name);

    int count_x; //read from qml
    int count_y; //read from qml

    QObject   *root;
};

#endif // CH_CHAR_H
