#!/bin/bash

function update_data()
{
	$TS/wav_scp.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1"
	echo "$1 wav.scp generated"
	$TS/utt2spk.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1"
	echo "$1 utt2spk generated"
	$TS/spk2g.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1/spk2gender"
	echo "$1 spk2gender generated"
	$TS/text.sh "$AUDIO_DIR/$1" "${DATA_DIR}/$1/text"
	echo "$1 text generated"
	$TS/corpus.sh "$AUDIO_DIR/$1" "${DATA_DIR}/local/corpus.txt"
	echo "$1 corpus generated"
	
	$SC/sort.sh "$DATA_DIR/$1/text"
	$SC/sort.sh "$DATA_DIR/$1/wav.scp"
	$SC/sort.sh "$DATA_DIR/$1/utt2spk"
	echo "--------- $1 finished ----------"
}

REC_TIME=5 #second
SC="bijan/scripts"
TS="bijan/train" #Train Script
AUDIO_DIR="digits_audio"
DATA_DIR="data"
SPEAKER="$1"
REC_NUM="$2"

STRING_DIGIT=(zero one two three four five six seven eight nine)

$TS/clean.sh

python3 $TS/recorder.py "$AUDIO_DIR" $SPEAKER $REC_NUM

update_data "train"
update_data "test"

$SC/sort.sh "$DATA_DIR/local/corpus.txt"

#./run.sh
	
