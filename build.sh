#!/bin/bash
set -eu

export CTEST_OUTPUT_ON_FAILURE=TRUE

cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug
make -C build "$@"

cp build/cli/can-simulator-ng .
cp build/gui/can-simulator-ng-gui .

if [ ! -f gui.conf ] ; then
    cp gui/gui.conf_template ./gui.conf
fi
