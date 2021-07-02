#!/bin/bash

export KALDI_DIR="/home/bijan/Project/ProjectZ/kaldi/"
WAV_DIR="bijan/wav"
OUT_DIR="bijan/audio"
REC_TIME=5 #second
REC_NAME="rec1.wav"

while true; do
	rm "$WAV_DIR/$REC_NAME"
	rm "$OUT_DIR/wav.scp"
	rm "$OUT_DIR/utt2spk"

	bijan/scripts/set_mic.sh
	python3 bijan/scripts/recorder.py "$WAV_DIR/$REC_NAME" $REC_TIME 2>/dev/null

	bijan/scripts/wav_scp.sh "$WAV_DIR" "$OUT_DIR"
	bijan/scripts/utt2spk.sh "$WAV_DIR" "$OUT_DIR"

	bijan/scripts/sort.sh "$OUT_DIR/wav.scp"
	bijan/scripts/sort.sh "$OUT_DIR/utt2spk"

	./becode.sh
done
