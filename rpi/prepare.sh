#!/bin/sh

# Get relevant submodules
git submodule update --init

# First pass, buildtoolchain an image
#cp gcui_defconfig buildroot/configs/gcui_defconfig
#cd buildroot/
#make gcui_defconfig
#make -j4
#cd ../

# Build raylib
cd raylib/src
make PLATFORM=PLATFORM_DRM USE_RPI_CROSS_COMPILER=TRUE RPI_TOOLCHAIN=../../buildroot/output/host/ RPI_TOOLCHAIN_NAME=arm-buildroot-linux-musleabihf RAYLIB_LIBTYPE=SHARED
cp ./libraylib.so* ../../buildroot/output/host/arm-buildroot-linux-musleabihf/sysroot/usr/lib
cp raylib.h ../../buildroot/output/host/arm-buildroot-linux-musleabihf/sysroot/usr/include/
cd ../../

# Build the app
cd ../
TOOLCHAIN_PATH=rpi/buildroot/output/host/ TOOLCHAIN_NAME=arm-buildroot-linux-musleabihf ./build_cross.sh 
cd rpi

# Install on target fs
cp raylib/src/libraylib.so* ../../buildroot/output/target/usr/lib
cp ../gcui ../reader ../../buildroot/output/target/usr/bin

# Make image
#cd buildroot/
#make -j4
