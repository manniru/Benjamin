#ifndef BTSTATE_H
#define BTSTATE_H

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
    char *cmvn_stat_path;

    // decoder
    int max_active;
    int min_active;
    int train_max;
    int min_sil;

    // captain
    double hard_threshold;
private:
    void load_config();
    void load_default();

};

#endif // BTSTATE_H
