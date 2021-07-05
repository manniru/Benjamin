#!/bin/bash

WAV_DIR="decode/wav"
REC_TIME=5 #second
REC_NAME="rec1.wav"

DECODE_PATH="decode"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"

source path.sh

while true; do
	$SD/clean.sh "$OUT_DIR" "$WAV_DIR" "$REC_NAME"
	
	$SD/record.sh "$WAV_DIR/$REC_NAME" $REC_TIME

	$SD/wav_scp.sh "$WAV_DIR" "$AUDIO_PATH"
	$SD/utt2spk.sh "$WAV_DIR" "$AUDIO_PATH"

	$SD/sort.sh "$AUDIO_PATH/wav.scp"
	$SD/sort.sh "$AUDIO_PATH/utt2spk"

	./decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH"
	
	#exit 0
done
