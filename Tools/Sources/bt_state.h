#ifndef BTSTATE_H
#define BTSTATE_H

#define BT_NORM_MODE 0     // online detection mode
#define BT_TEST_MODE 1     // read from wav file instead of online
#define BT_ENN_MODE  2     // extract features from train wave dataset

class BtState
{
public:
    BtState();
    int state;

private:

};

#endif // BTSTATE_H
