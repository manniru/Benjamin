#!/bin/bash
source path.sh

MODE="tri1"
# MODE="mono"

WORD_TABLE="exp/$MODE/graph/words.txt"
WORD_SIL='1:2:3:4:5:6:7:8:9:10'
HCLG_FST="exp/$MODE/graph/HCLG.fst"
FINAL_MDL="exp/$MODE/final.mdl"

LATGEN_OPTIONS="--max-active=7000 --beam=13.0 --acoustic-scale=0.05"
online-gmm-decode-faster $LATGEN_OPTIONS $FINAL_MDL $HCLG_FST $WORD_TABLE $WORD_SIL
