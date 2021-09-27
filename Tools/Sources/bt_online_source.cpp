#include "bt_online_source.h"

BtOnlineSource::BtOnlineSource(BtCyclic *buffer)
{
    cy_buf = buffer;
}

BtOnlineSource::~BtOnlineSource()
{
    ;
}

bool BtOnlineSource::Read(kaldi::Vector<float> *data)
{
    int req = data->Dim();  // number of samples to request
    int16_t *raw = (int16_t *)malloc(sizeof(int16_t)*req);

    while( true )
    {
        Pa_Sleep(2);
        if( req<cy_buf->getSize() )
        {
            break;
        }
    }

    int nsamples = cy_buf->raad(raw, req);
    data->Resize(nsamples);
//    qDebug() << req << nsamples;

    for( int i = 0 ; i<nsamples ; i++ )
    {
        (*data)(i) = raw[i];
    }
    free(raw);

    if( nsamples==0 )
    {
        return false;
    }

    return true;
}
