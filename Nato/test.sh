#!/bin/bash
source path.sh

DECODE_PATH="decode"
WAV_DIR="$DECODE_PATH/wav"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
LAT_CONF="$RESULT_PATH/lat_conf.ark"
#WORDS=$($SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH")

if [[ "$1" == "r" ]]; then

	echo "Rec Mode"
	REC_TIME="5"
	$SD/record.sh "$DECODE_PATH/wav/rec1.wav" $REC_TIME

elif [[ "$1" == "t" ]]; then

	echo "Test Mode"
    
    TEST_NAME=$(ls audio/test/)
    find "audio/test/$TEST_NAME" -type f > list_file
    sed -i '1d' list_file #remove first line

    while read p; do
	    
	    cp "$p" "$DECODE_PATH/wav/rec1.wav"
        $SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" 1
	    
		    
    done <list_file

    rm list_file
    exit 0

elif [[ "$1" == "v" ]]; then

	echo "Verify Mode"
    
    steps/decode.sh --config conf/decode.config --nj 1 exp/tri1/graph data/train exp/tri1/decode

fi

time $SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" 1
#time $SD/create_conf.sh decode 0.08 0.027
#$SK/visualize.sh "$LAT_CONF" "$RESULT_PATH" #visualize
#$SD/print_words.sh decode 0.9
#cat "$RESULT_PATH/confidence"
