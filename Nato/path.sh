# Defining Kaldi root directory
export KALDI_ROOT=`pwd`/../../KalB
# Setting paths to useful tools
export PATH=$PWD/utils/:$KALDI_ROOT/src/bin:$KALDI_ROOT/tools/openfst-1.7.2/bin:$KALDI_ROOT/src/fstbin/:$KALDI_ROOT/src/gmmbin/:$KALDI_ROOT/src/featbin/:$KALDI_ROOT/src/lmbin/:$KALDI_ROOT/src/sgmm2bin/:$KALDI_ROOT/src/fgmmbin/:$KALDI_ROOT/src/latbin/:$KALDI_ROOT/src/online2bin:$KALDI_ROOT/src/onlinebin:$PWD:$PATH
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH:-}:$KALDI_ROOT/tools/openfst-1.7.2/lib
# Variable needed for proper data sorting
export LC_ALL=C

SD="scripts/decode" #Script Decode
ST="scripts/train" #Script Train
SI="scripts/interpreter" #Script Interpreter
SK="scripts/kaldi" #Script Kaldi
