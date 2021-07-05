#!/bin/bash
. ./path.sh || exit 1

DECODE_PATH="$1"
AUDIO_PATH="$2"
RESULT_PATH="$3"

WORD_TABLE="exp/mono/graph/words.txt"
FINAL_MDL="exp/mono/final.mdl"
HCLG_FST="exp/mono/graph/HCLG.fst"

WAV_SCP="$AUDIO_PATH/wav.scp"
UTT2SPK="$AUDIO_PATH/utt2spk"
SPK2UTT="$AUDIO_PATH/spk2utt"
CMVN="$AUDIO_PATH/cmvn.scp"
FEAT="$AUDIO_PATH/feats.scp"

LATTICE="$RESULT_PATH/lattice.ark"
RESULT="$RESULT_PATH/best.tra"

LATGET_OPTIONS="--max-active=7000 --beam=13.0 --lattice-beam=6.0 --acoustic-scale=0.083333 --allow-partial=true --word-symbol-table=$WORD_TABLE $FINAL_MDL $HCLG_FST"

MFCC_DIR="$DECODE_PATH/mfcc"
LOG_DIR="$DECODE_PATH/log"

rm -rf $MFCC_DIR $SPK2UTT
rm -rf $AUDIO_PATH/* $LOG_DIR $LATTICE $RESULT_PATH

if [[ "$1" != "clean" ]]; then

	utils/utt2spk_to_spk2utt.pl $UTT2SPK > $SPK2UTT
	steps/make_mfcc.sh --nj 1 $AUDIO_PATH $LOG_DIR $MFCC_DIR > /dev/null
	steps/compute_cmvn_stats.sh $AUDIO_PATH $LOG_DIR $MFCC_DIR > /dev/null

	gmm-latgen-faster $LATGET_OPTIONS "ark,s,cs:apply-cmvn  --utt2spk=ark:$UTT2SPK scp:$CMVN scp:$FEAT ark:- | add-deltas  ark:- ark:- |" ark,t:$LATTICE  2>&1 | grep ^bijan | sed 's/[^ ]* *//'

fi
	
