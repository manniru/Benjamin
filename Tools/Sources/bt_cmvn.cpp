#include "bt_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>
#include <transform/cmvn.h>

using namespace kaldi;

BtCMVN::BtCMVN(QThread *thread, QObject *parent) : QObject(parent)
{
    cmvn_rspecifier = "scp:$CMVN";
    feat_rspecifier = "scp:$FEAT";
    feat_wspecifier = "ark:$LAT_CMVN";
    utt2spk_rspecifier = "ark:$UTT2SPK";
}

BtCMVN::~BtCMVN()
{
    ;
}

void BtCMVN::compute()
{
    bool norm_vars = false;

    kaldi::int32 num_done = 0, num_err = 0;

    SequentialBaseFloatMatrixReader feat_reader(feat_rspecifier);
    BaseFloatMatrixWriter feat_writer(feat_wspecifier);

    RspecifierType crt = ClassifyRspecifier(cmvn_rspecifier,
                                            NULL, NULL);

    if( crt!=kNoRspecifier )
    {
        // reading from a Table: per-speaker or per-utt CMN/CVN.
        RandomAccessDoubleMatrixReaderMapped cmvn_reader(cmvn_rspecifier,
                                                         utt2spk_rspecifier);

        while( !feat_reader.Done() )
        {
            feat_reader.Next();
            std::string utt = feat_reader.Key();
            Matrix<BaseFloat> feat(feat_reader.Value());

            if( !cmvn_reader.HasKey(utt) )
            {
                KALDI_WARN << "No normalization statistics available for key "
                           << utt << ", producing no output for this utterance";
                num_err++;
                continue;
            }

            Matrix<double> cmvn_stats = cmvn_reader.Value(utt);

            ApplyCmvn(cmvn_stats, norm_vars, &feat);
            feat_writer.Write(utt, feat);

            num_done++;
        }
    }
    else
    {
        qDebug() << "--utt2spk option not compatible with rxfilename as input ";
    }
}
