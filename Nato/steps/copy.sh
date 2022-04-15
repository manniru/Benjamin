#!/bin/bash

if [[ ! -e "../Tools/Model/" ]]; then
	mkdir "../Tools/Model/"
fi
cp "exp/tri1/graph/HCLG.fst" "../Tools/Model/HCLG.fst"
cp "exp/tri1/graph/words.txt" "../Tools/Model/words.txt"
cp "exp/tri1_online/final.oalimdl" "../Tools/Model/final.oalimdl"
cp "exp/tri1_online/global_cmvn.stats" "../Tools/Model/global_cmvn.stats"