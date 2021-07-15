# Benjamin
Bijan ASR Engine Based-on KalD

# Cygwin
Use autotools instead of CMake. Kaldi CMake is deprecated

```
apt-cyg install pulseaudio
```

# Benjamin
Bijan ASR Engine Based-on KalD

# Installation
Build Kaldi, Use Intel MKL for faster kaldi performance but OpenBLAS is a decent choice

To control the PC this project connect with *Rebound* and uses Linux uInput module.

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


