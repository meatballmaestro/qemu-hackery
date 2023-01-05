#! /bin/bash

PREFIX=$HOME/test/qemu

# Dependencies
## Python 2.7

wget https://www.python.org/ftp/python/2.7/Python-2.7.tgz
tar -zxvf Python-2.7.tgz 
cd Python-2.7/
./configure --prefix=$PREFIX
make
make install

## Gettext

sudo apt install gettext

# Qemu

mkdir build
cd build/
../configure --prefix=$PREFIX --python=$PREFIX/bin/python --disable-werror
make
make install
~/test/qemu/bin/qemu-system-x86_64 
