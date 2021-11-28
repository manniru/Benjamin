#ifndef BT_CYCLIC_H
#define BT_CYCLIC_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"
#include "matrix/kaldi-vector.h"

class BtCyclic : public QObject
{
    Q_OBJECT
public:
    explicit BtCyclic(int size, QObject *parent = nullptr);

    int  read(int16_t *data, int size);
    int  read(kaldi::Vector<float> *data, int size);
    void write(int16_t *data, int size);
    void constWrite(int16_t data, int size);
    void rewind(int count);
    int  getDataSize(); // Get Data size that are in Buf
    int  getFreeSize(); // Get Availible Byte to Write in Buf

private:
    int16_t *buffer;

    int read_p;
    int write_p;
    int buff_size;
};

#endif // BT_CYCLIC_H
