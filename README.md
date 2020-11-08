## Introduction

The purpose of this simulator is to send simulated vehicle
data through CAN bus. 

GUI is done with QT own libraries, its ugly.

## Directory structure

## Build and usage

Build can be done using the 'build.sh' script.
You can test this simulator with virtual CAN interface (vCAN).
CAN bus initialization can be done using for example './can_init.sh can0 500000'.
Basic usage of CAN simulator:
Available commands can be listed using
./can-simulator-ng --help
Adding permissions for automatic CAN bus initialization to can-simulator-ng binary
sudo setcap cap_net_raw,cap_net_admin+ep can-simulator-ng

## Unittesting
Run to compile and execute tests:

./build.sh tests test
Tests are implemented using googletest framework. All tests reside in tests subfolder.

## Dependencies
The CAN simulator is dependant on cmake, libsocketcan, libjansson and libcap to work
Installation: sudo apt-get install cmake libsocketcan-dev libjansson-dev libcap-dev
CAN Simulator UI also requires Qt libraries
Installation: sudo apt-get install qtbase5-dev

Building with unit tests (required for deb package builds):

sudo apt-get install libgtest-dev

mkdir -p /tmp/gtest-build
pushd /tmp/gtest-build
cmake -DCMAKE_BUILD_TYPE=RELEASE /usr/src/gtest/
make
sudo cp -a libg* /usr/lib/
popd


## Building deb package
Install requirements for deb package building:
sudo apt-get install debhelper
Build deb package:
dpkg-buildpackage -us -uc
