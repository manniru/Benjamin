# Installation

## Step 1: Clone Project
Clone Kaldi and Benjamin in the same folder

```
git clone https://github.com/bijanbina/KalB.git
git clone --recursive https://github.com/bijanbina/Benjamin.git
```

## Step 2: Build Kaldi with Intel MKL

* OpenBLAS is slow for the training

On Arch Linux

```
sudo pacman -S intel-mkl gcc-fortran subversion portaudio python-pyaudio bc
cd KalB/tools
make
cd ../src
./configure --mathlib=MKL
make
```

## Step 3: Train Your Model

Go to `NATO` directory and

1. Edit `word_list` file and write down words you want the system to detect.
2. Check that all words are existed in `Nato/scripts/lang/cmudict.dict`
3. Start recording samples by running

```
./record.sh <category> <number>
```

* `category`: any name so you can distinguish your environment or microphone later
* `number`: number of samples you want to record

4. Train the Model
```
./train.sh
```