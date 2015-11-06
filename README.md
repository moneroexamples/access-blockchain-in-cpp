# access-blockchain-in-cpp

Into

# Objective
What the example is doing.

# Pre-requsits

Everthing here was done and tested on
Ubuntu 14.04 x86_64 and Ubuntu 15.10 x86_64.

For monero node is using `lmdbq`, thus I use this database only in this example.
I could not test on `berkeleydb`, thus its not included here.

## Dependencies


```bash
# refresh ubuntu's repository
sudo apt-get update

#install git
sudo apt-get install git

# install dependencies
sudo apt-get install build-essential cmake libboost1.55-all-dev miniupnpc libunbound-dev graphviz doxygen libdb5.1++-dev
```

## Monero compilation


```bash
# download the latest bitmonero source code from github
git clone https://github.com/monero-project/bitmonero.git

# go into bitmonero folder
cd bitmonero/

make # or make -j number_of_threads, e.g., make -j 2
```

## Monero static libraries
When the compilation finishes, bunch of static monero libraries
should be generated. We will need to link against.

They will be spread around different subfolders of the `./build/` folder.
So it is easier to just copy them into one folder. I assume that
 `/opt/bitmonero-dev/libs` will be our folder where we are going to keep them.

```bash
# create the folder
sudo mkdir -p /opt/bitmonero-dev/libs

# find the static libraries files (i.e., those with extension of *.a)
# and copy them to /opt/bitmonero-dev/libs
# assuming you are still in bitmonero/ folder which got downloaded from
# github
sudo find ./build/ -name '*.a' -exec cp {} /opt/bitmonero-dev/libs  \;
 ```

## Monero headers

Now we need to get headers, as this is our interface to the
monero libraries.

```bash
# create the folder
sudo mkdir -p /opt/bitmonero-dev/headers

# find the header files (i.e., those with extension of *.h)
# and copy them to /opt/bitmonero-dev/headers
# but this time the structure of directions is important
# so rsync is used to find and copy the headers files
sudo rsync -zarv --include="*/" --include="*.h" --exclude="*" --prune-empty-dirs ./ /opt/bitmonero-dev/headers
 ```

## cmake confing files
`CMakeLists.txt` files and the structure of this project are can be studied at github. I wont be discussing them
here. I tried to put comments in `CMakeLists.txt` to clarify what is there.

The location of the Monero's headers and static libraries must be correctly
indicated in  `CMakeLists.txt`. So if you put them in different folder
that in this example, please change the root `CMakeLists.txt` file
to reflect this.

# C++11 code
The two most interesting C++11 source files in this example are `MicroCore.cpp` and `main.cpp`. Therefore, I will focus on them here.
Full source code is on github.

## MicroCore.cpp

Source doce

## main.cpp

Source code


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```
