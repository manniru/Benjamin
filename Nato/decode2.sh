#!/bin/bash

DECODE_PATH="decode"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
WAV_FILENAME="$1"
DBUS_PATH="--dest=com.binaee.batool / com.binaee.batool"

SC_CW="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SC_CW

source path.sh

ln -s -f $WAV_FILENAME $DECODE_PATH/wav/rec1.wav

WORDS=$($SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH")

$SD/create_conf.sh decode 0.05 0.027
dbus-send --session $DBUS_PATH.exec string:"$WORDS"
