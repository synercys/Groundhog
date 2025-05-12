# Groundhog: A Restart-based Framework for Increased Availability in High-Threshold Cryptosystems

Groundhog is a framework/mechanism to improve resiliency to node corruptions in threshold cryptosystems. This mechanism can be laid on top of various protocols used in threshold cryptosystem.
We demonstrate Groundhog on 2 protocols used in threshold cryptosystems : a) Distributed Symmetric Encryption (DiSE) and b) Boneh, Lynn and Shacham Distributed (BLS) Signatures

Groundhog works by restarting the nodes strategically by ensuring threshold number of nodes are available at anytime to perform the operations.

Requirements:
- `git` (or download and unzip this repo)
- `docker`
- `docker-compose`

```
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
```
Tested with Ubuntu 20.04 LTS and Debian 11.
If the distro has `cmake` version 3.18 or newer available in the package repository, you can just install that package and delete the CMake part of the script.
