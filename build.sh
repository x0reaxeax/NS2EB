#!/bin/bash

# Set EDK2 environment
export EDK2_PATH=$PWD/edk2
source $EDK2_PATH/edksetup.sh

# Add your package to the build path
export PACKAGES_PATH=$PWD:$EDK2_PATH

# Explicit use of NASM
export NASM_PREFIX=nasm

# Build your package
build -a X64 -t GCC5 -p NS2EB-Pkg/NS2EB.dsc
