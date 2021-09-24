#ifndef BT_CYCLIC_H
#define BT_CYCLIC_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"

class BtCyclic : public QObject
{
    Q_OBJECT
public:
    explicit BtCyclic(int size, QObject *parent = nullptr);

    int  raad(int16_t *data, int size);
    void write(int16_t *data, int size);
    void constWrite(int16_t data, int size);
    void rewind(int count);
    int  getSize();

private:
    int16_t *buffer;

    int read_p;
    int write_p;
    int buff_size;
};

#endif // BT_CYCLIC_H
