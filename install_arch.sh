#!/bin/bash
# Bijan Binaee 2021 <bijan-at-binaee-dot-com>

git clone https://github.com/kaldi-asr/kaldi.git Kaldi
git clone https://github.com/bijanbina/Benjamin.git

#sudo pacman -S lapack
sudo pacman -S intel-mkl gcc-fortran subversion portaudio python-pyaudio
cd Kaldi/tools
extras/check_dependencies.sh
extras/install_mkl.sh
extras/install_srilm.sh <name> <organization> <email>
make
cd ../src
./configure --mathlib=MKL
make

cd ../..
cp Kaldi/egs/wsj/s5 Benjamin/NATO/steps

cd Benjamin/Tools
qmake ./BaTool.pro
make
