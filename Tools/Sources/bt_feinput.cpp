#include "bt_feinput.h"

using namespace kaldi;

BtFeInput::BtFeInput(BtCyclic *buf, QObject *parent): QObject(parent)
{
    rec_buf = buf;
    o_features = new BtCFB;
    frame_num = 0;
    mfcc = new KdMFCC;
    delta = new KdDelta(o_features);
    cmvn = new BtCMVN(o_features);
}

void BtFeInput::resetCmvn()
{
    //    cmvn->resetStat();
}

int BtFeInput::Dim()
{
    int mfcc_dim = mfcc->Dim();
    return mfcc_dim * (1 + BT_DELTA_ORDER);
}

int BtFeInput::NumFramesReady()
{
    int offset = BT_DELTA_ORDER * BT_DELTA_ORDER; //4
    int ret     = frame_num - offset;
    if( ret>0 )
    {
        return ret;
    }
    return 0;
}

// Called from outside(decodable)
void BtFeInput::GetFrame(int frame, Vector<float> *feat)
{
    int context = BT_DELTA_ORDER * KD_DELTA_WINDOW; //4
    int left_frame = frame - context;
    int right_frame = frame + context + 1;
    if( left_frame<0 )
    {
        left_frame = 0;
    }
    if( right_frame>=frame_num )
    {
        right_frame = frame_num - 1;
    }
    if( right_frame<left_frame )
    {
        qDebug() << "right_frame" << right_frame
                 << "left_frame"  << left_frame;
        exit(1);
    }
    for( int i=left_frame ; i<right_frame ; i++ )
    {
        cmvn->calc(i);
    }
    delta->Process(frame, frame_num);
    o_features->writeFeat(frame, feat);
}

BtFeInput::~BtFeInput()
{
    // Guard against double deleting the cmvn_ ptr
    delete cmvn;
    delete mfcc;
}

void BtFeInput::ComputeFeatures()
{
    KdWindow &window_cal = mfcc->frame_opts;
    rec_buf->rewind(remain_samp);
    int len = rec_buf->getDataSize()-10;
    int frame_length = window_cal.WindowSize();
    int frame_count = window_cal.frameCount(len);
    if( len<frame_length )
    {
        remain_samp = 0;
        return;
    }

    int ws = window_cal.WindowShift(); //window shift
    int read_len = (frame_count-1)*ws+frame_length;

    wav_buf.Resize(read_len);
    rec_buf->read(&wav_buf, read_len);

    Vector<float> window;
    for( int i=0 ; i<frame_count ; i++)
    {
        window_cal.extract(i*ws, wav_buf, &window);

        Vector<float> *features = new Vector<float>(mfcc->Dim(),
                                                    kUndefined);

        mfcc->Compute(&window, features);
        o_features->writeVec(frame_num, features);
        frame_num++;
    }
    remain_samp = frame_length-ws;
}
