GCC=arm-xilinx-linux-gnueabi-gcc
all: 
	$(GCC) main.c bitbang-spi.c -o main --std=c99

