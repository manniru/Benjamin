#!/bin/bash
DECODE_PATH="decode"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"

source path.sh

$SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH"

$SD/create_conf.sh decode 0.08 0.03
cat $RESULT_PATH/confidence
