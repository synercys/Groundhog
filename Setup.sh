sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libssl-dev
wget https://github.com/Kitware/CMake/releases/download/v3.18.5/cmake-3.18.5.tar.gz
tar -xf cmake-3.18.5.tar.gz
cd cmake-3.18.5
./bootstrap
make -j 4
sudo make install 
cd ..
mkdir Redise 
cd Redise
git clone https://github.com/disha-agarwal/dise.git
git clone https://github.com/relic-toolkit/relic.git
git clone https://github.com/ladnir/cryptoTools
cd relic (arm - use presets)
cmake . -D MULTI=PTHREAD
make -j
sudo make install
cd ../cryptoTools/thirdparty/linux
bash boost.get
cd ../..
cmake . -D ENABLE_RELIC=ON 
make -j
cd ../dise
cmake .
make -j
./bin/dEncFrontent -u
https://howchoo.com/g/ndy1zte2yjn/how-to-set-up-wifi-on-your-raspberry-pi-without-ethernet#navigate-to-the-boot-directory
