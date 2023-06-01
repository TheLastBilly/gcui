#!/bin/sh

TOOLCHAIN_SYSROOT=${TOOLCHAIN_PATH}/${TOOLCHAIN_NAME}/sysroot

CC=${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_NAME}-cc

LDFLAGS=-L${TOOLCHAIN_SYSROOT}/usr/lib/
INCLUDE_PATH=${TOOLCHAIN_SYSROOT}/usr/include/

${CC} -o gcui gcui.c bt.c -Wl,-rpath=${TOOLCHAIN_SYSROOT} -I${INCLUDE_PATH} -lGLESv2 -lEGL -lm -ldl -lrt -lraylib -lc 

${CC} reader.c -o reader
