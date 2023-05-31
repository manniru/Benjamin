#ifndef CH_CHAR_H
#define CH_CHAR_H

#include <QObject>
#include <windows.h> // for key codes
#include "backend.h"
#include "config.h"

class ChChar : public QObject
{
    Q_OBJECT
public:
    ChChar(QObject *ui, QObject *parent = NULL);
    ~ChChar();

    int strToPos(QChar input);
private:
    void addCell(QString name);
    void createCells();

    int count_x; //read from qml
    int count_y; //read from qml
    QVector<int> dictionary;

    QObject   *root;
};

#endif // CH_CHAR_H
