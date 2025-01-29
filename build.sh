#!/bin/bash

set -e  # Stop the script if any command fails

BUILD_TARGET=${1:-DEBUG}

# Set EDK2 environment
export EDK2_PATH=$PWD/edk2
export WORKSPACE=$PWD
export PACKAGES_PATH=$WORKSPACE:$EDK2_PATH

# Ensure BaseTools is built
if [ ! -d "$EDK2_PATH/BaseTools/Source/C" ]; then
    echo "Building BaseTools..."
    make -C $EDK2_PATH/BaseTools
fi

# Source the EDK2 environment
pushd $EDK2_PATH > /dev/null
. edksetup.sh BaseTools
popd > /dev/null

# Build NS2EB
build -a X64 -t GCC5 -b $BUILD_TARGET -p NS2EB-Pkg/NS2EB.dsc
