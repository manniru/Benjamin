# Installation

## Step 1: Clone Project
Clone Kaldi and Benjamin in the same folder

```
git clone https://github.com/kaldi-asr/kaldi.git Kaldi
cd Kaldi
git checkout 579c9bf1770698f1383cb1cea5528d36fd8e7b93
cd ..
git clone --recursive https://github.com/bijanbina/Benjamin.git
```

## Step 2: Build Kaldi with Intel MKL

* OpenBLAS is slow for the training

On Arch Linux

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

## Step 3: Train Your Model

Go to `NATO` directory and

1. Edit `word_list` file and write down words you want the system to detect.
2. Check that all words are existed in `Nato/scripts/lang/cmudict.dict`
3. Start recording samples by running

```
./record.sh <category> <number>
```

`category`: any name so you can distinguish your environment or microphone later
`number`: number of samples you want to record

4. Create Lexicon (must run if the word_list changed)

```
./lang_word.sh
```

5. Train the Model
```
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