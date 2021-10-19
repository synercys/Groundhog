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

Note: This project works on a specific commit for cryptotools-
We can execute the following in the $BUILD_DIR/cryptoTools/ 

'''
git checkout 851e388f8301a7e20aaa6ca5f5a57d609c2b8158
'''

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


### To run Baseline:
We can run this setup with a minimum of 3 devices setup as given above. 
'''
cd $BUILD_DIR/dise/
git checkout 913a6fdd57e4b5e775fb69c8a5c551402f5f18b2
'''

For now the ips of the 3 nodes are set here : https://github.com/disha-agarwal/dise/blob/913a6fdd57e4b5e775fb69c8a5c551402f5f18b2/test/Helloworld.cpp#L14 
Ips of the n nodes should be set in  $BUILD_DIR/dise/test/Helloworld.cpp. (refer to the link above).
 
To Run- `./bin/test`
