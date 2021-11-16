# htDiSE

## Build Instructions

### Docker

Either directly run `docker run gabrielkulp/htdise`, or build the image yourself with `docker build .`.

You can minimize the image size by `docker export`ing and then `docker import`ing, which will discard the history of the image, shrinking it from over 400MB to around 15.

### VM or Baremetal

Download and run [setup.sh](Setup.sh):

```sh
wget https://raw.githubusercontent.com/disha-agarwal/dise/master/Setup.sh
sh Setup.sh
```

Tested with Ubuntu 20.04 LTS and Debian 11. If building for another distro, you might need to change the first few `apt` commands. If the distro has `cmake` version 3.18 or newer available in the package repository, you can just install that package and delete the CMake part of the script.

## Execution Instuctions
We can run this setup with a minimum of 3 devices setup as given above.
The IPs of the 3 nodes are set in [test/Helloworld.cpp](test/Helloworld.cpp#L14) on line 14.

Then on each device, run `./bin/test`.
