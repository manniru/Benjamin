#!/bin/bash
source path.sh

DECODE_PATH="decode"
WAV_DIR="$DECODE_PATH/wav"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
LAT_CONF="$RESULT_PATH/lat_conf.ark"
#WORDS=$($SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH")

if [[ "$1" == "rec" ]]; then
	echo "Rec Mode"
	REC_TIME="5"
	$SD/record.sh "$DECODE_PATH/wav/rec1.wav" $REC_TIME
fi

$SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH"
time $SD/create_conf.sh decode 0.08 0.027
$SK/visualize.sh "$LAT_CONF" "$RESULT_PATH" #visualize
#$SD/print_words.sh decode 0.9
