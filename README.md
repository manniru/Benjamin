# Benjamin
Bijan ASR Engine Based-on KalD

# Installation

## Step 1: Build Kakdi
Use Intel MKL for faster kaldi performance but OpenBLAS is also a decent choice

### Arch Linux
```
git clone https://github.com/kaldi-asr/kaldi.git
sudo pacman -S lapack python-pyaudio
extras/install_openblas.sh
cd tools
make
./install_srilm.sh
cd ../src
./configure --mathlib=OPENBLAS
```

### Windows Cygwin
Use autotools instead of CMake. Kaldi CMake is deprecated

```
apt-cyg install pulseaudio
```

## Step 2: Import Neccesary Tools

1. From `kaldi/egs/wsj/s5` copy `utils` and `steps` into Nato Project. 
2. Copy `kaldi/egs/voxforge/s5/local/score.sh` to `Nato/local` 


# Project Strurcture

- **Digits**: Example project implementing zero to nine recognition
- **Nato**: This directory contain scripts to implement any low vacabulary ASR system
- **Tools**: Any nessasry tool to assist the detection process. 

----------------------
To control the PC this project connect with [Rebound](https://github.com/bijanbina/RAIIS) and uses Linux uInput module.

