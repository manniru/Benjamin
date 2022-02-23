# Benjamin
Bijan ASR Engine Based-on KalD

[YouTube Demo](https://youtu.be/aPQmxTXUgmA)

# Installation

## Step 1: Prepare Kakdi
1. Clone Kaldi and Benjamin in the same folder

```
git clone https://github.com/kaldi-asr/kaldi.git Kaldi
git clone https://github.com/bijanbina/Benjamin.git
```

2. Build Kaldi with Intel MKL. On Arch Linux

```
sudo pacman -S intel-mkl gcc-fortran subversion portaudio
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

   From `Kaldi/egs/wsj/s5` copy `utils` and `steps` into Nato Project.

2. Train the model. These bash scripts are available for the NATO task

```
# Record Train Audio Data
./train.sh <category> <number>
# Train Model
./run.sh
# Create Lexicon (must run if the word_list changed)
./lang_word.sh
```

## Step 4: Compile and Run BaTool

use qmake to compile BaTool and Execute

# Project Strurcture

- **Digits**: Example project implementing zero to nine recognition
- **Nato**: This directory contain scripts to implement any low vacabulary ASR system
- **Tools**: Any nessasry tool to assist the detection process. 

### Windows Cygwin
Use autotools instead of CMake. Kaldi CMake is deprecated

```
apt-cyg install portaudio
```

----------------------
* To control the PC this project connect with [Rebound](https://github.com/bijanbina/RAIIS) and uses Linux uInput module.
* OpenBLAS is slower and doesn't work with this projectgit clone https://github.com/kaldi-asr/kaldi.git
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
* For Arch Linux follow `install_arch.sh` to find sample commands

```
sudo pacman -S python-pyaudio
```
