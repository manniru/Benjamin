# Installation

## Step 1: Prepare Kakdi
1. Clone Kaldi and Benjamin in the same folder

```
git clone https://github.com/kaldi-asr/kaldi.git Kaldi
cd Kaldi
git checkout 579c9bf1770698f1383cb1cea5528d36fd8e7b93
cd ..
git clone --recursive https://github.com/bijanbina/Benjamin.git
```

2. Build Kaldi with Intel MKL. On Arch Linux

* OpenBLAS is slow for the training

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

## Step 2: Train Your Model

Bash scripts available for the NATO task:

```
# Record Audio Data
./record.sh <category> <number>
# Create Lexicon (must run if the word_list changed)
./lang_word.sh
# Train the Model
./train.sh
```

----------

### Windows Cygwin
Use autotools instead of CMake. Kaldi CMake is deprecated

```
apt-cyg install portaudio
```

* Look `steps` and `utils` inside `Kaldi/egs/wsj/s5` for complete set of tools.
* For Arch Linux simply run `install_arch.sh`