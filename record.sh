#!/bin/bash

WAV_DIR="bijan/wav"
OUT_DIR="bijan/audio"
REC_TIME=5 #second
REC_NAME="rec1.wav"
SC="bijan/scripts"

while true; do
	$SC/clean.sh "$OUT_DIR" "$WAV_DIR" "$REC_NAME"
	
	$SC/record.sh "$WAV_DIR/$REC_NAME" $REC_TIME

	$SC/wav_scp.sh "$WAV_DIR" "$OUT_DIR"
	$SC/utt2spk.sh "$WAV_DIR" "$OUT_DIR"

	$SC/sort.sh "$OUT_DIR/wav.scp"
	$SC/sort.sh "$OUT_DIR/utt2spk"

	./becode.sh
	
	#exit 0
done
