#!/bin/bash

function update_data()
{
	$ST/w_u2s.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1"
	echo "$1 wav.scp and utt2spk generated"
	$ST/spk2g.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1/spk2gender"
	echo "$1 spk2gender generated"
	$ST/ct.sh "$AUDIO_DIR/$1" "${DATA_DIR}" "$1"
	echo "$1 text and corpus generated"
	
	$SD/sort.sh "$DATA_DIR/$1/text"
	$SD/sort.sh "$DATA_DIR/$1/wav.scp"
	$SD/sort.sh "$DATA_DIR/$1/utt2spk"
	$SD/sort.sh "$DATA_DIR/$1/spk2gender"
	echo "--------- $1 finished ----------"
}

REC_TIME=5 #second
AUDIO_DIR="audio"
DATA_DIR="data"
SPEAKER="$1"
REC_NUM="$2"

source path.sh

LEXICON_COUNT=$(wc -l word_list | awk '{ print $1 }')
LEXICON_COUNT=$(($LEXICON_COUNT-1))

$ST/clean.sh

python3 $ST/recorder.py "$AUDIO_DIR" $SPEAKER $REC_NUM $LEXICON_COUNT

update_data "train"
update_data "test"

$SD/sort.sh "$DATA_DIR/local/corpus.txt"

$ST/check_audio.sh $LEXICON_COUNT

#./run.sh
	
