#include "bt_online_source.h"

BtOnlineSource::BtOnlineSource()
{
    ;
}

BtOnlineSource::~BtOnlineSource()
{
    ;
}

bool BtOnlineSource::Read(kaldi::Vector<float> *data)
{
    uint32 nsamples_req = data->Dim();  // samples to request

    std::vector<int16> buf(nsamples_req);
    long nsamples_rcv;
    nsamples_rcv = 1;//PaUtil_ReadRingBuffer(&pa_ringbuf_, buf.data(), nsamples_req);

    data->Resize(nsamples_rcv);
    for (int i = 0; i < nsamples_rcv; ++i)
    {
        (*data)(i) = static_cast<float>(buf[i]);
    }

    return (nsamples_rcv != 0);
    // NOTE (Dan): I'm pretty sure this return value is not right, it could be
    // this way because we're waiting.  Vassil or someone will have to figure this
    // out.
}
