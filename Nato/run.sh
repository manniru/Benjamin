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

AUDIO_DIR="audio"
DATA_DIR="data"

source path.sh
$ST/clean.sh

update_data "train"
update_data "test"

$SD/sort.sh "$DATA_DIR/local/corpus.txt"

$ST/run.sh
