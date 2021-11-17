#!/bin/sh

# If using Ubuntu, clear out some unneeded services that slow
# down booting. Only do this if the VM will only run HTDiSE
sudo -v || exit 1
sudo apt -y purge snapd lxd cloud-init apparmor
sudo apt -y autoremove

# Update the system and install build dependencies
sudo apt -y update
sudo apt -y upgrade
sudo apt -y install git build-essential libssl-dev python-is-python3
#apk add git gcc g++ make cmake python3 bash linux-headers openssl-dev

# Set the directory where everything will be cloned and built
PRJ_DIR="${HOME}/Redise"
mkdir -p "$PRJ_DIR"


# If CMake is too old in the repositories (need at least 3.18)
# then we need to download, build, and install it. Here's how:
cd "$PRJ_DIR"
#CMAKE_V=3.18.5
CMAKE_V=3.21.4
wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_V}/cmake-${CMAKE_V}.tar.gz
tar -xf cmake-${CMAKE_V}.tar.gz
cd cmake-${CMAKE_V} || exit 2
./bootstrap
make -j `nproc`
sudo make install


# This project depends on cryptoTools,
# so we need to build it from source with cmake
cd "$PRJ_DIR"
git clone https://github.com/ladnir/cryptoTools
cd cryptoTools || exit 3
#git checkout f5c8e4f411b7a4ba17423ff8afe63e58918faffe # old stable
git checkout ca471904c3c49ee4ae6101efcbb22869337e3b7e # recent stable
python build.py --setup --boost --relic
python build.py -- -D ENABLE_RELIC=ON
python build.py --install --sudo


# And now clone this repo and build it
cd "$PRJ_DIR"
git clone https://github.com/disha-agarwal/dise.git
cd dise || exit 4
cmake .
make -j `nproc`

# Finally, run a quick test
./bin/dEncFrontend -u

echo "Done!"


# Notes for using with Raspberry Pi devices:

# https://howchoo.com/g/ndy1zte2yjn/how-to-set-up-wifi-on-your-raspberry-pi-without-ethernet#navigate-to-the-boot-directory
# 
# ARM
# relic build using presets
# cmake -DARCH=ARM -DWSIZE=32 -DMULTI=PTHREAD
# 
# cryptotools(redise/cryptoTools/CMakeLists.txt)
# option(ENABLE_SSE       "compile with SSE instrctions" OFF)
# 
# /home/ubuntu/redise/cryptoTools/cryptoTools/CMakeLists.txt
# target_compile_options(cryptoTools PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++14> -pthread -latomic)
# target_link_options(cryptoTools PUBLIC -pthread -latomic)
# 
# dise 
# /home/ubuntu/redise/dise/CMakeLists.txt
# remove -maes -msse2 -msse4.1 -mpclmul flags
# set(CRYPTOTOOLS_DIR "${CMAKE_SOURCE_DIR}/../cryptoTools" CACHE STRING "location of cryptoTools root")
