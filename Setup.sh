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
cd relic 
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

ARM
relic build using presets
cmake -DARCH=ARM -DWSIZE=32 -DMULTI=PTHREAD

cryptotools(redise/cryptoTools/CMakeLists.txt)
option(ENABLE_SSE       "compile with SSE instrctions" OFF)

/home/ubuntu/redise/cryptoTools/cryptoTools/CMakeLists.txt
target_compile_options(cryptoTools PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++14> -pthread -latomic)
target_link_options(cryptoTools PUBLIC -pthread -latomic)

dise 
/home/ubuntu/redise/dise/CMakeLists.txt
remove -maes -msse2 -msse4.1 -mpclmul flags
