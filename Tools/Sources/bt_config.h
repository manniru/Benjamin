#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#define DBUS_NAME "com.binaee.batool"

#define KAL_NATO_DIR      "../Nato/"
#define KAL_DECODE_DIR    KAL_NATO_DIR"decode/"
#define KAL_SI_DIR        KAL_NATO_DIR"scripts/interpreter/"
#define KAL_SD_DIR        KAL_NATO_DIR"scripts/decode/"
#define KAL_RESULT_DIR    KAL_DECODE_DIR"result/"
#define KAL_WAV_DIR       KAL_DECODE_DIR"wav/"
#define KAL_MODE          "tri1"

#define BT_WORDS_PATH     KAL_NATO_DIR"exp/"KAL_MODE"/graph/words.txt"
#define BT_CONF_PATH      KAL_RESULT_DIR"confidence"

#define KAL_CONF_TRESHOLD  0.9
#define KAL_UDET_TRESHOLD  0.7 //Utterance Total Detection
#define KAL_UCON_TRESHOLD  0.9 //Utterance Total Confidence
#define KAL_HARD_TRESHOLD  0.5 //word will be removed

#define BT_REC_RATE       16000 //Recording Sample Rate
#define BT_BUF_SIZE       10    //Buffer Size Second
#define BT_REC_SIZE       3     //Record Size Second
#define BT_DEC_TIMEOUT    1     //Decode Timeout in Second
#define BT_REC_INPUT      "alsa:usbstream:CARD=Solo" //Microphone Input

#endif // BT_CONFIG_H
