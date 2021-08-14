#!/usr/bin/env bash

mfcc_config=conf/mfcc.conf
data=$1

compute-mfcc-feats --verbose=2 --config=$mfcc_config scp:$data/wav.scp ark:1.ark
copy-feats --compress=true ark:1.ark ark,scp:$data/feats.ark,$data/feats.scp
compute-cmvn-stats --spk2utt=ark:$data/spk2utt scp:$data/feats.scp ark,scp:$data/cmvn.ark,$data/cmvn.scp


# Store frame_shift and mfcc_config along with features.
frame_shift=$(perl -ne 'if (/^--frame-shift=(\d+)/) {
                          printf "%.3f", 0.001 * $1; exit; }' $mfcc_config)
echo ${frame_shift:-'0.01'} > $data/frame_shift

