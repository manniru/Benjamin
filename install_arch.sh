#!/bin/bash

git clone https://github.com/kaldi-asr/kaldi.git
mv kaldi Kaldi #Rename kaldi to uppercase
#sudo pacman -S lapack
sudo pacman -S intel-mkl gcc-fortran subversion
cd Kaldi/tools
#extras/install_mkl.sh
extras/install_srilm.sh <name> <organization> <email>
make
cd ../src
./configure --mathlib=MKL
make
