#!/bin/bash

REC_TIME=5 #second
AUDIO_DIR="audio"
DATA_DIR="data"
SPEAKER="$1"
REC_NUM="$2"

source path.sh

LEXICON_COUNT=$(wc -l word_list | awk '{ print $1 }')
LEXICON_COUNT=$(($LEXICON_COUNT-1))

python3 $ST/recorder.py "$AUDIO_DIR" $SPEAKER $REC_NUM $LEXICON_COUNT
$ST/check_audio.sh $LEXICON_COUNT
