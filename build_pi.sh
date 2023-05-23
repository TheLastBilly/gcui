#!/bin/sh

gcc gcui.c bt.c -lraylib -lGL -lEGL -ldrm -lgbm -lm -lpthread -ldl -lrt -lX11 -Wall -o gcui

gcc reader.c -o reader