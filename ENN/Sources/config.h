#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#define KAL_NATO_DIR    "../Nato/"
#define KAL_AU_DIR      KAL_NATO_DIR "audio/"
#define BT_WORDS_PATH   KAL_NATO_DIR "word_list"

#define ENN_EPOCH_LOG   5
//#define ENN_TARGET_LOSS 0.04
#define ENN_TARGET_LOSS .5
#define ENN_WRONG_LOSS  1
#define ENN_MAX_EPOCH   200
#define ENN_LEARN_RATE  "0.001"

#define AB_WSL_ROOT     "Arch"

//#define ENN_IMAGE_DATASET

#endif // BT_CONFIG_H
