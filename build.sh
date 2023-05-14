#!/bin/sh

gcc gcui.c bt.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wall -o gcui
