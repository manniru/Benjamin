# Benjamin
Bijan ASR Engine Based-on KalD

[YouTube Demo](https://youtu.be/aPQmxTXUgmA)

# Installation

## Step 1: Prepare Kakdi
1. Clone kaldi in the same folder as Benjamin
2. Build Kaldi with Intel MKL exclusively (OpenBLAS is slower and doesn't work with this project)

### Arch Linux
```
git clone https://github.com/kaldi-asr/kaldi.git
sudo pacman -S lapack
cd tools
extras/install_mkl.sh
make
./install_srilm.sh
cd ../src
./configure --mathlib=MKL
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

```
sudo pacman -S python-pyaudio
```