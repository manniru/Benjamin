# Defining Kaldi root directory
export KALDI_ROOT=`pwd`/../../Kaldi
# Setting paths to useful tools
export PATH=$PWD/utils/:$KALDI_ROOT/src/bin:$KALDI_ROOT/tools/openfst/bin:$KALDI_ROOT/src/fstbin/:$KALDI_ROOT/src/gmmbin/:$KALDI_ROOT/src/featbin/:$KALDI_ROOT/src/lmbin/:$KALDI_ROOT/src/sgmm2bin/:$KALDI_ROOT/src/fgmmbin/:$KALDI_ROOT/src/latbin/:$KALDI_ROOT/src/online2bin:$KALDI_ROOT/src/onlinebin:$PWD:$PATH
. $KALDI_ROOT/tools/env.sh
# Variable needed for proper data sorting
export LC_ALL=C

SD="scripts/decode" #Script Decode
ST="scripts/train" #Script Train
SI="scripts/interpreter" #Script Interpreter
