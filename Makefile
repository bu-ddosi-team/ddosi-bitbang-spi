GCC=arm-xilinx-linux-gnueabi-gcc
#GCC=gcc
CFLAGS= --std=c99 -Iinclude
SRC_DIR=src
all:
	$(GCC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/ddosi/bitbang-spi.c -o main

clean:
	rm main