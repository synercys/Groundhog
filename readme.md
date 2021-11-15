# htDiSE

## Build Instructions

Run [setup.sh](Setup.sh) on a recent Debian-based distro, or follow these equivalent instructions manually:

### System Setup

If using Ubuntu, clear out some unneeded services that slow down booting:

```sh
sudo apt -y purge snapd lxd cloud-init apparmor
sudo apt -y autoremove
```

Update the system and install important build dependencies

```sh
sudo apt -y update
sudo apt -y upgrade
sudo apt -y install build-essential libssl-dev python-is-python3
```

Choose the directory where everything will be cloned and built:

```sh
PRJ_DIR = "${HOME}/Redise"
mkdir -p "$PRJ_DIR"
```

### Get Recent `cmake`

Next, we'll need a newer version of `cmake` than what's in the repositories.
So let's download, build, and install it:

```sh
cd "PRJ_DIR"
CMAKE_V=3.21.4 # tested functional, most recent stable
wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_V}/cmake-${CMAKE_V}.tar.gz
tar -xf cmake-${CMAKE_V}.tar.gz
cd cmake-${CMAKE_V}
./bootstrap
make -j `nproc`
sudo make install
```

### Install cryptoTools (and Boost and Relic)

This project depends on [cryptoTools](https://github.com/ladnir/cryptoTools), so we need to build it (with `cmake) from source:

```sh
cd "PRJ_DIR"
git clone https://github.com/ladnir/cryptoTools
cd cryptoTools
git checkout ca471904c3c49ee4ae6101efcbb22869337e3b7e # up to date
python build.py --setup --boost --relic
python build.py -- -D ENABLE_RELIC=ON
python build.py --install --sudo
```

### Build HTDiSE

And now we can build this project!

```sh
cd "PRJ_DIR"
git clone https://github.com/disha-agarwal/dise.git
cd dise
cmake .
make -j `nproc`
```

Finally, test that everything worked correctly with `./bin/dEncFrontend -u`.

### To run Baseline:
We can run this setup with a minimum of 3 devices setup as given above.
For now the IPs of the 3 nodes are set in [test/Helloworld.cpp](test/Helloworld.cpp#L14) on line 14.

Then on each device, run `./bin/test`.
