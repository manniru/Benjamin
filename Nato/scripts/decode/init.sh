#!/bin/bash
REC_NAME="rec1.wav"

DECODE_PATH="decode"
WAV_DIR="$DECODE_PATH/wav"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
UTT2SPK="$AUDIO_PATH/utt2spk"
SPK2UTT="$AUDIO_PATH/spk2utt"

SC_CW="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SC_CW/../.. #cd to nato project

source path.sh

$SD/clean.sh "$AUDIO_PATH" "$WAV_DIR" "$DECODE_PATH"

touch "$WAV_DIR/$REC_NAME" #create fake wav file

$SD/wav_scp.sh "$WAV_DIR" "$AUDIO_PATH"
$SD/utt2spk.sh "$WAV_DIR" "$AUDIO_PATH"

$SD/sort.sh "$AUDIO_PATH/wav.scp"
$SD/sort.sh "$AUDIO_PATH/utt2spk"

rm "$WAV_DIR/$REC_NAME" #remove fake wav file

utils/utt2spk_to_spk2utt.pl $UTT2SPK > $SPK2UTT

mkdir -p $AUDIO_PATH/conf
cp conf/mfcc.conf $AUDIO_PATH/conf/mfcc.conf
