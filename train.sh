#!/bin/bash

REC_TIME=5 #second
SC="bijan/scripts"
TS="bijan/train" #Train Script
AUDIO_DIR="digits_audio"
DATA_DIR="data"

STRING_DIGIT=(zero one two three four five six seven eight nine)

$TS/clean.sh

python3 $TS/recorder.py "$AUDIO_DIR"

$SC/wav_scp.sh "$AUDIO_DIR/train" "${DATA_DIR}/train"
$SC/wav_scp.sh "$AUDIO_DIR/test" "${DATA_DIR}/test"
$SC/utt2spk.sh "$AUDIO_DIR/train" "${DATA_DIR}/train"
$SC/utt2spk.sh "$AUDIO_DIR/test" "${DATA_DIR}/test"
$TS/spk2g.sh "$AUDIO_DIR/train" "${DATA_DIR}/train/spk2gender"
$TS/spk2g.sh "$AUDIO_DIR/test" "${DATA_DIR}/test/spk2gender"
$TS/text.sh "$AUDIO_DIR/train" "${DATA_DIR}/train/text"
$TS/text.sh "$AUDIO_DIR/test" "${DATA_DIR}/test/text"
$TS/corpus.sh "$AUDIO_DIR/train" "${DATA_DIR}/local/corpus.txt"
$TS/corpus.sh "$AUDIO_DIR/test" "${DATA_DIR}/local/corpus.txt"

$SC/sort.sh "$DATA_DIR/train/text"
$SC/sort.sh "$DATA_DIR/test/text"
$SC/sort.sh "$DATA_DIR/local/corpus.txt"
$SC/sort.sh "$DATA_DIR/train/wav.scp"
$SC/sort.sh "$DATA_DIR/test/wav.scp"
$SC/sort.sh "$DATA_DIR/train/utt2spk"
$SC/sort.sh "$DATA_DIR/test/utt2spk"

#./run.sh
	
