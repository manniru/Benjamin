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

    void raad(int16_t *data, int size);
    void write(int16_t *data, int size);
    void constWrite(int16_t data, int size);
    void rewind(int count);

private:
    int16_t *buffer;

    int read_p;
    int write_p;
    int buff_size;

    double buff_data_size;
};

#endif // BT_CYCLIC_H
