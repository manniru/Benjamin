#!/bin/bash

DECODE_PATH="decode"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
WAV_FILENAME="$1"
DBUS_BATOOL="--dest=com.binaee.batool / com.binaee.batool"
DBUS_REBOND="--dest=com.binaee.rebound / com.binaee.rebound"

SC_CW="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SC_CW

source path.sh

ln -s -f $WAV_FILENAME $DECODE_PATH/wav/rec1.wav

$SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH"

$SD/create_conf.sh decode 0.05 0.027
dbus-send --session $DBUS_BATOOL.exec string:""
