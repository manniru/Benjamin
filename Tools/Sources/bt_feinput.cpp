#include "bt_feinput.h"

BtFeInput::BtFeInput(BtCyclic *buf, QObject *parent): QObject(parent)
{
    rec_buf = buf;
    o_features = new BtCFB;
    frame_num = 0;
    mfcc = new BtMFCC;
    delta = new KdDelta(o_features);
    cmvn = new BtCMVN(o_features);
}

uint BtFeInput::NumFramesReady()
{
    uint offset = BT_DELTA_ORDER * BT_DELTA_ORDER; //4
    int ret     = frame_num - offset;
    if( ret>0 )
    {
        return frame_num - offset;
    }
    return 0;
}

// Called from outside(decodable)
void BtFeInput::computeFrame(uint frame)
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
        qDebug() << "Error 1: rframe" << right_frame
                 << "lframe" << left_frame
                 << "frame:" << frame;
        exit(1);
    }
    for( int i=left_frame ; i<right_frame ; i++ )
    {
        cmvn->calc(i);
    }
    delta->Process(frame, frame_num);
}

BtFeInput::~BtFeInput()
{
    // Guard against double deleting the cmvn_ ptr
    delete cmvn;
    delete mfcc;
}

void BtFeInput::ComputeFeatures()
{
    int len = rec_buf->getDataSize()-10;
    int frame_length = mfcc->win.frameLen();
    int frame_count  = mfcc->win.frameCount(len);
    if( len<frame_length )
    {
        remain_samp = 0;
        return;
    }

    int ws = mfcc->win.frameShift(); //window shift
    int rewind_samp = frame_length-ws;

    for( int i=0 ; i<frame_count ; i++)
    {
        rec_buf->read(window_buf, frame_length);
        // add zero padded data
        for( int i=frame_length ; i<BT_FFT_SIZE ; i++ )
        {
            window_buf[i] = 0;
        }
        rec_buf->rewind(rewind_samp);

        mfcc->win.ProcessWindow(window_buf);

        BtFrameBuf *buf = o_features->get(frame_num);
        mfcc->Compute(window_buf, buf);
        frame_num++;
    }
}
