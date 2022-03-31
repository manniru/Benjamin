#! /bin/sh
#verify train data
#t_verify.sh

steps/decode.sh --config conf/decode.config --nj 1 exp/tri1/graph data/train exp/tri1/decode