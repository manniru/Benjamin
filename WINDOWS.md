- You need Windows 11 or Windows 10 version 2004 (Build 19041) or higher.

1. Open CMD with administrator and execute:
```
wsl --install
```
2. Reboot
3. Download [Arch.zip](https://github.com/yuk7/ArchWSL/releases/download/22.3.18.0/Arch.zip) and extract it under `C:\Arch`

4. Open `Arch.exe` and execute:

```
nano /etc/pacman.conf
# Add at bottom
[archlinuxfr]
SigLevel = Never
Server = http://repo.archlinux.fr/$arch

```


```
pacman-key --init; pacman-key --populate
pacman-key --refresh-keys
pacman -Sy archlinux-keyring
pacman -Syyu
pacman -S git make wget sox automake autoconf patch unzip bc
```

[More Info: Arch Linux WSL2](https://gist.github.com/ld100/3376435a4bb62ca0906b0cff9de4f94b)