# Installation

## Step 1: Prepare Kakdi
1. Clone Kaldi and Benjamin in the same folder

```
git clone https://github.com/kaldi-asr/kaldi.git Kaldi
git clone https://github.com/bijanbina/Benjamin.git
```

2. Build Kaldi with Intel MKL. On Arch Linux

```
sudo pacman -S intel-mkl gcc-fortran subversion portaudio python-pyaudio
cd Kaldi/tools
extras/check_dependencies.sh
extras/install_mkl.sh
extras/install_srilm.sh <name> <organization> <email>
make
cd ../src
./configure --mathlib=MKL
make
```

## Step 2: Prepare Model

1. Import Neccesary Tools.

   From `Kaldi/egs/wsj/s5` copy `steps` into Nato Project.

2. Train the model. These bash scripts are available for the NATO task

```
# Record Train Audio Data
./train.sh <category> <number>
# Train Model
./run.sh
# Create Lexicon (must run if the word_list changed)
./lang_word.sh
```

## Step 3: Compile and Run BaTool

use qmake to compile BaTool and Execute


### Windows Cygwin
Use autotools instead of CMake. Kaldi CMake is deprecated

```
apt-cyg install portaudio
```
