# htDiSE
'
```
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libssl-dev
```

## Build Instructions
This project works for a particular cmake version. Instructions for installing is as follows :

```
wget https://github.com/Kitware/CMake/releases/download/v3.18.5/cmake-3.18.5.tar.gz
tar -xf cmake-3.18.5.tar.gz
cd cmake-3.18.5
./bootstrap
make -j 4
sudo make install 
```

### Part 1: clone the dependencies               
Set the parent directory that we will build in
```
git clone https://github.com/visa/dise.git
git clone https://github.com/relic-toolkit/relic.git
git clone https://github.com/ladnir/cryptoTools
```

We require the code has the following structure 
```
$BUILD_DIR/cryptoTools/
$BUILD_DIR/dise/
```

### Part 2: Build and install Relic              

```
cd relic

cmake . -D MULTI=PTHREAD
make -j
sudo make install
```

On windows you can build relic with `-D MULTI=OPENMP`.

Note, you can install to a non-sudo location by calling `make DESTDIR=<path/to/install> install`


### Part 3: build boost and cryptoTools          
```
cd ../cryptoTools/thirdparty/linux
bash boost.get
cd ../..
cmake . -D ENABLE_RELIC=ON 
make -j
```


### Part 4: build DiSE                           
```
cd ../dise
cmake .
make -j
```

Run the unit tests `./bin/dEncFrontent -u`.
