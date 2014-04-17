#build for ARM
GCC=arm-xilinx-linux-gnueabi-gcc
#GCC=gcc
#Include static so we don't have to worry about libraries
CFLAGS= --std=c99 -Iinclude -static
SRC_DIR=src
all:
	$(GCC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/ddosi/bitbang-spi.c -o bin/main

clean:
	rm bin/main
