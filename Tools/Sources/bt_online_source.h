#ifndef BT_ONLINE_SOURCE_H
#define BT_ONLINE_SOURCE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>

#include "matrix/kaldi-vector.h"
#include "online/online-audio-source.h"

class BtOnlineSource: public kaldi::OnlineAudioSourceItf
{
public:
    BtOnlineSource();

    // Implementation of the OnlineAudioSourceItf
    bool Read(kaldi::Vector<float> *data);

    ~BtOnlineSource();

private:

};


#endif // BT_ONLINE_SOURCE_H
