#!/bin/bash
. ./path.sh

if [[ $# -gt 2 ]]; then

	DECODE_PATH="$1"
	AUDIO_PATH="$2"
	RESULT_PATH="$3"

else

	echo "Decode Test Mode"
	DECODE_PATH="decode"
	AUDIO_PATH="data/test"
	RESULT_PATH="$DECODE_PATH/result"

fi

MODE="tri1"
# MODE="mono"

WORD_TABLE="exp/$MODE/graph/words.txt"
HCLG_FST="exp/$MODE/graph/HCLG.fst"
FINAL_MDL="exp/$MODE/final.mdl"

WAV_SCP="$AUDIO_PATH/wav.scp"
CMVN="$AUDIO_PATH/cmvn.scp"
FEAT="$AUDIO_PATH/feats.scp"
UTT2SPK="$AUDIO_PATH/utt2spk"

LATTICE_FILE="ark:$RESULT_PATH/lattice.ark"
LAT_DELTA="$RESULT_PATH/deltas.ark"
LAT_CMVN="$RESULT_PATH/cmvn.ark"

LATGEN_OPTIONS="--max-active=7000 --beam=13.0 --lattice-beam=6.0 --acoustic-scale=0.083333 --allow-partial=false --word-symbol-table=$WORD_TABLE $FINAL_MDL $HCLG_FST"

START=$(date +%s%3N)
$SD/make_mfcc.sh $AUDIO_PATH 2>/dev/null
END=$(date +%s%3N)
MFCC_TIME=$(echo "$END - $START" | bc)

START=$(date +%s%3N)
apply-cmvn --utt2spk=ark:$UTT2SPK scp:$CMVN scp:$FEAT ark:$LAT_CMVN 2>/dev/null
END=$(date +%s%3N)
CMVN_TIME=$(echo "$END - $START" | bc)

START=$(date +%s%3N)
add-deltas ark:$LAT_CMVN ark:$LAT_DELTA 2>/dev/null
END=$(date +%s%3N)
DELTA_TIME=$(echo "$END - $START" | bc)

if [[ $# -lt 2 ]] || [[ $# -gt 3 ]]; then
	START=$(date +%s%3N)
	gmm-latgen-faster $LATGEN_OPTIONS ark:$LAT_DELTA $LATTICE_FILE  2>&1 | grep ^bijan
	END=$(date +%s%3N)
	GMM_TIME=$(echo "$END - $START" | bc)
	
	echo M=$MFCC_TIME C=$CMVN_TIME D=$DELTA_TIME G=$GMM_TIME
fi


