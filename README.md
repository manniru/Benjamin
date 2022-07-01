# Benjamin
Bijan ASR Engine Based-on KalD Model.

[YouTube Demo](https://youtu.be/aPQmxTXUgmA)

# Installation

1. If this is your first time using Benjamin, You need to train a model from your voice. Please follow [NATO.md](https://github.com/bijanbina/Benjamin/blob/master/NATO.md) to create one.
2. Build BaTool. Follow [INSTALL.md](https://github.com/bijanbina/Benjamin/blob/master/INSTALL.md) guide
* Only use release source code or commit that tagged with `stable version`. the main branch is in development and may not work.

# Features

- Zero delay output
- Fast and lightweight
- Portable and easy to install

# Project Strurcture

- **Digits**: Example project implementing zero to nine recognition.
- **ENN**: Ehsan Nueral Network, Used to calculate confidence scores.
- **Nato**: NATO task used to create a low vacabulary ASR Model based on CMVN and Delta-Delta Tri-Phone Algorithm.
- **Tools**: BaTool project do the realtime detection.

# Aknolegdment

This project wouldn't be possible without the help of:
- M.P. Zanoosi
- S. Dadashi
- Dr. E. Akhavan
- M. Abdollah Zadeh
- Tiny-DNN
- OpenFST
- HTKBook
- Kaldi
- [rxi-ini](https://github.com/rxi/ini)

----------------------
* To control the PC this project connect with [Rebound](https://github.com/bijanbina/RAIIS/tree/master/Rebound) and uses Linux uInput module.
