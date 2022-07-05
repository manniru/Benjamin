#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#define ENN_FALSE_COUNT 10
#define ENN_TRAIN_DIR   "../Nato/audio/enn/"
#define KAL_NATO_DIR    "../Nato/"
#define BT_WORDS_PATH   KAL_NATO_DIR "word_list"

#define ENN_EPOCH_LOG   25
//#define ENN_TARGET_LOSS 0.04
#define ENN_TARGET_LOSS 0.5
#define ENN_MIN_LRATE   0.0002
#define ENN_MAX_EPOCH   200
#define ENN_LEARN_RATE  0.001

#define BT_ENN_SIZE     40

//#define ENN_IMAGE_DATASET

#endif // BT_CONFIG_H
