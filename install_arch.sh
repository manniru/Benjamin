#!/bin/bash
# Bijan Binaee 2021 <bijan-a/t-binaee-d0t-com>

SR_EMAIL=""
SR_NAME=""
SR_ORG=""

git clone https://github.com/kaldi-asr/kaldi.git Kaldi
git clone --recursive https://github.com/bijanbina/Benjamin.git

sudo pacman -S intel-mkl gcc-fortran subversion portaudio python-pyaudio
cd Kaldi/tools
extras/check_dependencies.sh
extras/install_mkl.sh
extras/install_srilm.sh "$SR_NAME" "$SR_EMAIL" "$SR_ORG"
make
cd ../src
./configure --mathlib=MKL
make

cd ../..
cp Kaldi/egs/wsj/s5 Benjamin/NATO/steps
