#! /bin/sh
#verify train data
#if run with any arg -> run on test data
#./tr_verify.sh

if [[ "$#" == "1" ]]; then
	echo "Verifing test data"
	steps/decode.sh --config conf/decode.config --nj 1 exp/tri1/graph data/test exp/tri1/decode
else
	echo "Verifing train data"
	steps/decode.sh --config conf/decode.config --nj 1 exp/tri1/graph data/train exp/tri1/decode
fi