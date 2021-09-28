# Benjamin
Bijan ASR Engine Based-on KalD

[YouTube Demo](https://youtu.be/aPQmxTXUgmA)

# Installation

## Step 1: Prepare Kakdi
1. Clone Kaldi in the same folder as Benjamin
2. Build Kaldi with Intel MKL exclusively (OpenBLAS is slower and doesn't work with this project)

* For Arch Linux follow `install_arch.sh` to find sample commands

## Step 2: Import Neccesary Tools

1. From `kaldi/egs/wsj/s5` copy `utils` and `steps` into Nato Project.
2. Copy `kaldi/egs/voxforge/s5/local/score.sh` to `Nato/local`

## Step 3: Prepare Model

These bash scripts are available for the NATO task

```
# Record Train Audio Data
./train.sh <category> <number>
# Train Model
./run.sh
# Create Lexicon (must run if the word_list changed)
./lang_word.sh
```

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
To control the PC this project connect with [Rebound](https://github.com/bijanbina/RAIIS) and uses Linux uInput module.

```
sudo pacman -S python-pyaudio
```
