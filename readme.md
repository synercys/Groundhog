# Groundhog: A Restart-based Framework for Increased Availability in High-Threshold Cryptosystems

Groundhog is a framework/mechanism to improve resiliency to node corruptions in threshold cryptosystems. This mechanism can be laid on top of various protocols used in threshold cryptosystem.
We demonstrate Groundhog on 2 protocols used in threshold cryptosystems : a) Distributed Symmetric Encryption (DiSE) and b) Boneh, Lynn and Shacham Distributed (BLS) Signatures

Groundhog works by restarting the nodes strategically by ensuring threshold number of nodes are available at anytime to perform the operations.

## Execution Instructions
Simply run `docker-compose up` to start a swarm of containers.
We can run this setup with a minimum of 3 nodes.

If you do not have the container locally (and don't want to build it), you must run `docker pull gabrielkulp/htdise` first to download a pre-built version.

[`docker-compose.yml`](docker-compose.yml) defines the setup.
The `scale` option on line 9 must match the `-n` option on line 8.
This is the number of nodes.

`-mc` lets you choose the threshold as an absolute count and `-t` determines the number of trials to run for statistics.

## Build Instructions

### Docker

Requirements:
- `git` (or download and unzip this repo)
- `docker`
- `docker-compose`

git clone https://github.com/synercys/Groundhog.git
cd Groundhog
# Remove the tracked directory

git rm -r cryptoTools
rm -rf cryptoTools  # Just in case

# From your repo
git submodule add https://github.com/ladnir/cryptoTools.git cryptoTools
cd cryptoTools
git checkout 76c2aff1b7c8001a701c5c045204252c34df7f8c

# Go back to root and commit the submodule pointer
cd ../
git add .gitmodules cryptoTools

Tested with Ubuntu 20.04 LTS and Debian 11. If building for another distro, you might need to change the first few `apt` commands. If the distro has `cmake` version 3.18 or newer available in the package repository, you can just install that package and delete the CMake part of the script.
