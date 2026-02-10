#!/bin/bash

# Build MicroPython-LVGL app for: Unix/Linux systems

source env-variables-micropython.sh

VARIANT=lvgl

cd $MICROPYTHON
make -C mpy-cross

cd $MICROPYTHON/ports/unix


make VARIANT=$VARIANT clean

make VARIANT=$VARIANT -j

#  colormake PROG=libmicropython.so USER_C_MODULES=../../user_modules/ -j