#!/bin/bash

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

LAT_CONF="$RESULT_PATH/lat_conf.ark"
VIS_ARK="$RESULT_PATH/vis.ark"
VIS_FST="$RESULT_PATH/vis.fst"
VIS_BIN="$RESULT_PATH/vis.bin"
VIS_PDF="$RESULT_PATH/vis.pdf"
LAT_DELTA="$RESULT_PATH/deltas.ark"
LAT_ALIGN="$RESULT_PATH/lat_align.ark"
CONF_FILE="$RESULT_PATH/confidence"

LATGEN_OPTIONS="--max-active=7000 --beam=15.0 --lattice-beam=8.0 --acoustic-scale=$ACS_LATGEN --determinize-lattice=true --allow-partial=false --word-symbol-table=$WORD_TABLE $FINAL_MDL $HCLG_FST"

FRAME_SHIFT=$(cat $AUD_PATH/frame_shift)
CONF_OPT="--frame-shift=$FRAME_SHIFT --acoustic-scale=$ACS_CONF --decode-mbr=true"

gmm-latgen-faster $LATGEN_OPTIONS ark:$LAT_DELTA ark:$LAT_CONF  2>/dev/null
#lattice-align-words $WORD_BOUNDARY $FINAL_MDL ark:$LAT_CONF ark:$LAT_ALIGN 2>/dev/null
lattice-to-ctm-conf $CONF_OPT ark:$LAT_CONF $CONF_FILE 2>/dev/null

if [[ "$4"=="vis" ]]; then
    lattice-to-fst --acoustic-scale=0.00005 ark:$LAT_CONF ark,t:$VIS_ARK 2>/dev/null
    tail -n +2 $VIS_ARK > $VIS_FST #remove first line
    
    #round weight to 3 decimal
    AWK_CMD='{printf("%s\t%s\t%s\t%s\t%.3f\n", $1, $2, $3, $4, $5)}'

    rm "$VIS_ARK"
	while read line; do
		
		LINE_CNT=$(echo "$line" | wc -w)
		if [[ "$LINE_CNT" == "5" ]]; then
		    echo "$line" | awk "$AWK_CMD" >> "$VIS_ARK"
		else
		    echo "$line" >> "$VIS_ARK"
		fi
		
	done <$VIS_FST
    
    fstcompile $VIS_ARK $VIS_BIN
    fstdraw --portrait=true --isymbols=$WORD_TABLE --acceptor=true $VIS_BIN | dot -Tpdf > $VIS_PDF
fi

