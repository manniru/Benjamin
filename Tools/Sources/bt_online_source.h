#ifndef BT_ONLINE_SOURCE_H
#define BT_ONLINE_SOURCE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>

#include "online/online-audio-source.h"
#include "matrix/kaldi-vector.h"
#include "bt_cyclic.h"

class BtOnlineSource: public kaldi::OnlineAudioSourceItf
{
public:
    BtOnlineSource(BtCyclic *buffer);

    // Implementation of the OnlineAudioSourceItf
    bool Read(kaldi::Vector<float> *data);

    ~BtOnlineSource();

private:
    BtCyclic *cy_buf;
};


#endif // BT_ONLINE_SOURCE_H
