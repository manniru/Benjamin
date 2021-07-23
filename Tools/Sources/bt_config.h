#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#define DBUS_NAME "com.binaee.batool"

#define KAL_NATO_DIR      "../Nato/"
#define KAL_DECODE_DIR    KAL_NATO_DIR"decode/"
#define KAL_SI_DIR        KAL_NATO_DIR"scripts/interpreter/"
#define KAL_RESULT_DIR    KAL_DECODE_DIR"result/"
#define KAL_MODE          "tri1"

#define BT_WORDS_PATH     KAL_NATO_DIR"exp/"KAL_MODE"/graph/words.txt"
#define BT_CONF_PATH      KAL_RESULT_DIR"confidence"

#define KAL_CONF_TRESHOLD 0.9
#define KAL_UTT_TRESHOLD  0.7
#define KAL_HARD_TRESHOLD 0.5 //word will be removed

#endif // BT_CONFIG_H
