#!/usr/bin/env bash

. ./path.sh

DECODE_PATH=$1
ACS_LATGEN=$2
ACS_CONF=$3


RESULT_PATH="$DECODE_PATH/result"
AUD_PATH="$DECODE_PATH/audio"

MODE="tri1"
# MODE="mono"

WORD_TABLE="exp/$MODE/graph/words.txt"
HCLG_FST="exp/$MODE/graph/HCLG.fst"
WORD_BOUNDARY="exp/$MODE/graph/phones/word_boundary.int"
FINAL_MDL="exp/$MODE/final.mdl"

LAT_FILE="$RESULT_PATH/lattice.ark"
LAT_DELTA="$RESULT_PATH/deltas.ark"

LATGEN_OPTIONS="--max-active=7000 --beam=13.0 --lattice-beam=6.0 --acoustic-scale=$ACS_LATGEN --determinize-lattice=true --allow-partial=true --word-symbol-table=$WORD_TABLE $FINAL_MDL $HCLG_FST"

FRAME_SHIFT=$(cat $AUD_PATH/frame_shift)
CONF_OPT="--frame-shift=$FRAME_SHIFT --acoustic-scale=$ACS_CONF --decode-mbr=true"

gmm-latgen-faster $LATGEN_OPTIONS ark:$LAT_DELTA ark:$LAT_FILE  2>/dev/null
lattice-align-words $WORD_BOUNDARY $FINAL_MDL ark:$LAT_FILE ark:2.ark 2>/dev/null
lattice-to-ctm-conf $CONF_OPT ark:2.ark scores.conf 2>/dev/null

while read f; do

	INDEX=$(echo $f | awk '{print $5}')
	WORD=$(grep $INDEX $WORD_TABLE | awk '{print $1}')
	printf "%6s " $WORD

done <scores.conf

printf "\n"

while read f; do

	CONF=$(echo $f | awk '{print $6}')
	printf "%6s " $CONF

done <scores.conf

printf "\n"
