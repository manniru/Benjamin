#ifndef KD_ONLINE2_H
#define KD_ONLINE2_H

#include "bt_config.h"

//#ifdef BT_ONLINE2

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "kd_online2_gmm.h"
#include "bt_state.h"
#include "bt_recorder.h"
#include "bt_captain.h" //For BtWord
#include "kd_mbr.h"
#include "backend.h"

class KdOnline2 : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline2(QObject *parent = nullptr);
    ~KdOnline2();

    void startDecode();
    void processData(int len);

public slots:
    void init();

signals:
    void startRecord();
    void resultReady(QVector<BtWord> result);

private:
    void parseWords(QString filename);
    void execute(std::vector<int32_t> word);

    void print(kaldi::CompactLattice *clat);

    QVector<QString>  history;
    QVector<QString>  lexicon;
    KdOnline2Gmm     *g_decoder;
    QThread          *rec_thread;
    BtRecorder       *rec_src;
    BtCyclic         *cy_buf;
};

#endif

//#endif // KD_ONLINE2_H
