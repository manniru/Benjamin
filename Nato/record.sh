#!/bin/bash

REC_TIME=5 #second
REC_NAME="rec1.wav"

DECODE_PATH="decode"
WAV_DIR="$DECODE_PATH/wav"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
UTT2SPK="$AUDIO_PATH/utt2spk"
SPK2UTT="$AUDIO_PATH/spk2utt"

DBUS_PATH="--dest=com.binaee.batool / com.binaee.batool"

source path.sh

$SD/clean.sh "$AUDIO_PATH" "$WAV_DIR" "$DECODE_PATH"

touch "$WAV_DIR/$REC_NAME" #create fake wav file

$SD/wav_scp.sh "$WAV_DIR" "$AUDIO_PATH"
$SD/utt2spk.sh "$WAV_DIR" "$AUDIO_PATH"

$SD/sort.sh "$AUDIO_PATH/wav.scp"
$SD/sort.sh "$AUDIO_PATH/utt2spk"

utils/utt2spk_to_spk2utt.pl $UTT2SPK > $SPK2UTT

while true; do

	$SD/record.sh "$WAV_DIR/$REC_NAME" $REC_TIME
	$SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" show_result

	#$SD/create_conf.sh decode 0.05 0.027
	echo "$WORDS"

	if [[ "$#" -gt "0" ]];then 
		exit 0
	fi

done
