#!/bin/bash

OUT_DIR="$1"
WAV_DIR="$2"
REC_NAME="$3"

if [ -f "$OUT_DIR/wav.scp" ]; then
	rm "$OUT_DIR/wav.scp"
fi

if [ -f "$OUT_DIR/utt2spk" ]; then
	rm "$OUT_DIR/utt2spk"
fi

if [ -f "$WAV_DIR/$REC_NAME" ]; then
	rm "$WAV_DIR/$REC_NAME"
fi
