#!/bin/bash

git clone https://github.com/kaldi-asr/kaldi.git
sudo pacman -S lapack
cd tools
extras/install_mkl.sh
make
./install_srilm.sh
cd ../src
./configure --mathlib=MKL
