#!/bin/bash
source path.sh

MODE="tri3"
# MODE="mono"

WORD_TABLE="exp/$MODE/graph/words.txt"
HCLG_FST="exp/$MODE/graph/HCLG.fst"
OUT_LAT="ark:online/lattice.ark"
WAV_SCP="online/wav.scp"
SPK2UTT="online/spk2utt"
FINAL_MDL="exp/$MODE/final.mdl"

LATGEN_OPTIONS="--config=exp/tri3_online/conf/online_decoding.conf --max-active=7000 --beam=12.0 --lattice-beam=6.0 --acoustic-scale=0.05 --word-symbol-table=$WORD_TABLE --model=$FINAL_MDL"
online2-wav-gmm-latgen-faster $LATGEN_OPTIONS $HCLG_FST ark:$SPK2UTT scp:$WAV_SCP $OUT_LAT
