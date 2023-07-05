#ifndef BT_STATE_H
#define BT_STATE_H

#include <QString>
#include "config.h"

#define BT_NORM_MODE 0     // online detection mode
#define BT_TEST_MODE 1     // read from wav file instead of online
#define BT_ENN_MODE  2     // extract features from train wave dataset

class BtState
{
public:
    BtState();
    int state;

    // model
    char *fst_path;
    char *mdl_path;
    char *word_path;
    char *cmvn_stat_path;
    QString channel_np; // named pipe debug channel
    QString mic_name;

    // decoder
    int max_active;
    int min_active;
    int train_max;
    int min_sil;
    double ac_scale;

    // captain
    double hard_threshold;
private:
    void load_config();
    void load_default();

};

#endif // BT_STATE_H
