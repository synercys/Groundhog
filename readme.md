# htDiSE

## Build Instructions

### Docker

Either directly run `docker run gabrielkulp/htdise`, or clone/download this repository and build the image yourself with `docker build .`.

### VM or Baremetal

Download and run [setup.sh](Setup.sh):

```sh
wget https://raw.githubusercontent.com/disha-agarwal/dise/master/Setup.sh
sh Setup.sh
```

Tested with Ubuntu 20.04 LTS and Debian 11. If building for another distro, you might need to change the first few `apt` commands. If the distro has `cmake` version 3.18 or newer available in the package repository, you can just install that package and delete the CMake part of the script.

## Execution Instuctions
We can run this setup with a minimum of 3 devices.
The IPs of the 3 nodes are set in [test/Helloworld.cpp](test/Helloworld.cpp#L14) on line 14. TODO: specify IP addresses on the commandline.

Then on each device, run `./bin/test` (or `/usr/loca/bin/test` on the Docker images).
