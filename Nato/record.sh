#!/bin/bash
#record train audio file

REC_TIME=5 #second
AUDIO_DIR="audio"
DATA_DIR="data"
SPEAKER="$1"
REC_NUM="$2"

source path.sh

LEXICON_COUNT=$(wc -l word_list | awk '{ print $1 }')
LEXICON_COUNT=$(($LEXICON_COUNT-1))

#send go sleep to BaTool
DBUS_PATH="--dest=com.binaee.rebound / com.binaee.rebound"
dbus-send --session $DBUS_PATH.meta string:"6"
dbus-send --session $DBUS_PATH.apps  string:"7"

if [[ ! -d audio ]]; then

	mkdir audio
	mkdir audio/train
	mkdir audio/test

fi

if [[ ! -d audio/train/$SPEAKER ]]; then

	mkdir audio/train/$SPEAKER

fi

python3 $ST/recorder.py "$AUDIO_DIR" $SPEAKER $REC_NUM $LEXICON_COUNT
python3 $ST/check_audio.py
