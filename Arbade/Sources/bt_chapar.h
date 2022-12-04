#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_recorder.h"
#include "bt_wav_writer.h"
#include "backend.h"

class BtChapar : public QObject
{
    Q_OBJECT
public:
    explicit BtChapar(QObject *parent = nullptr);
    ~BtChapar();

    void record(QString category);

signals:
    void startDecoding();

private slots:
    void writeWav();

private:

    QStringList lexicon;
    BtRecorder *rec;
    BtWavWriter *wav;
    QVector<BtWord> words;
    int gui_len;
};

#endif // BT_CHAPAR_H
