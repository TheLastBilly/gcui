#!/bin/sh

# Get relevant submodules
git submodule update --init

# First pass, buildtoolchain an image
cp gcui_defconfig buildroot/configs/gcui_defconfig
cd buildroot/
make gcui_defconfig
make -j4
cd ../

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
cp raylib/src/libraylib.so* buildroot/output/target/usr/lib
cp ../gcui ../reader buildroot/output/target/usr/bin

# Configure image
echo "start_file=start.elf
fixup_file=fixup.dat
kernel=zImage
disable_overscan=1
gpu_mem=100
dtoverlay=miniuart-bt,krnbt=on
dtoverlay=vc4-kms-v3d" > buildroot/output/images/rpi-firmware/config.txt

echo "# /etc/inittab
::sysinit:/bin/mount -t proc proc /proc
::sysinit:/bin/mount -o remount,rw /
::sysinit:/bin/mkdir -p /dev/pts /dev/shm
::sysinit:/bin/mount -a
::sysinit:/sbin/swapon -a
null::sysinit:/bin/ln -sf /proc/self/fd /dev/fd
null::sysinit:/bin/ln -sf /proc/self/fd/0 /dev/stdin
null::sysinit:/bin/ln -sf /proc/self/fd/1 /dev/stdout
null::sysinit:/bin/ln -sf /proc/self/fd/2 /dev/stderr
::sysinit:/bin/hostname -F /etc/hostname

::sysinit:/etc/init.d/rcS

console::respawn:/sbin/getty -L  console 0 vt100 # GENERIC_SERIAL
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console
::sysinit:nohup reader -s /dev/ttyACM0 -p /tmp &
::sysinit:nohup gcui -x 1280 -y 720 -s /tmp -f &

::shutdown:/etc/init.d/rcK
::shutdown:/sbin/swapoff -a
::shutdown:/bin/umount -a -r" > buildroot/output/target/etc/inittab 

# Make image
cd buildroot/
make -j4
