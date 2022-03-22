#include "kd_online.h"

using namespace kaldi;
using namespace fst;

AmDiagGmm        *am_gmm;
TransitionModel  *trans_model;
KdOnlineLDecoder *o_decoder;
KdOnline2Model   *o2_model; //gaussain online 2 model

KdOnline::KdOnline(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    ab_src = new BtRecorder(cy_buf);
    status.word_count = 0;

    std::string model_rxfilename = BT_OAMDL_PATH;

    trans_model = new TransitionModel;
    am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    std::string online_alimdl = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    o2_model = new KdOnline2Model(trans_model, am_gmm, online_alimdl);

    o_decoder = new KdOnlineLDecoder(*trans_model);
}

KdOnline::~KdOnline()
{
    delete am_gmm;
    delete trans_model;
    delete o2_model;
    delete o_decoder;
}

void KdOnline::init()
{
    startDecode();
}

void KdOnline::startDecode()
{
    float acoustic_scale = 0.05;

    KdDecodable decodable(ab_src, o2_model,
                             acoustic_scale);

    ab_src->startStream();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

    while( 1 )
    {
        decodable.features->AcceptWaveform(cy_buf);
        o_decoder->Decode();
        result = o_decoder->getResult(&out_fst);
        processResult(result);

        if( o_decoder->status.state!=KD_STATE_NORMAL )
        {
            status.word_count = 0;
            emit reset();
        }
    }
}

void KdOnline::processResult(QVector<BtWord> result)
{
    if( result.empty() )
    {
        return;
    }

    QVector<BtWord> buf;

    for( int i=0 ; i<result.size() ; i++ )
    {
        if( i==0 )
        {
            if( result[i].conf<0.75 &&
                result.size()<3 )
            {
                continue;
            }
            if( result[i].end<0.15 )
            {
                continue;
            }
        }
        result[i].time += o_decoder->frame_num/100.0;
        buf += result[i];
    }
    emit resultReady(buf);
}
