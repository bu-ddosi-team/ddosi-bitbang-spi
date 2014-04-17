/**
 * main.h : bitbang spi interface for AD9910
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: April 6, 2014
 * Team 19 dDOSI
 * Boston Unversity ECE Department Senior Design
 * Boston University Electronics Design Facility.
 */

#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <time.h>
#include "ddosi/bitbang-spi.h"
#include "ddosi/ddosi-constants.h"
#include <sys/mman.h> //mmap/munmap
#include <fcntl.h>    //PROT_READ, PROT_WRITE
#include <unistd.h>   //device File IO

#define DDS_FS (1.0E9)

//The GPIO bus address for the DDS signals
unsigned long int PORT_ADDR=GPIO0_PORT_ADDR;

//Must allocate in increments of full blocks. The block size
//for the RAM on the microZed is 4K, seems wasteful for 4 bytes,
//but that's how it goes.
#define MAP_SIZE 4096UL

//#define NONARM_TEST

int main() {
	int enabled_channels = DDS_CH1;

	//Open the RAM and map port to it.
#ifdef NONARM_TEST
	int a = 0;
	void *port = (void *)&a;
#else
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd == -1) {
		printf("Unable to open memory.\n");
		return -1;
	}

	//This makes it so this address is read/writeable,
	//MAP_SHARED allows other programs to see the mapping.
	//See man page of mmap for more info.
	void *port = mmap(0, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PORT_ADDR);
	if (port == MAP_FAILED) {
		printf("Unable to map port to memory.\n");
		close(fd);
		return -1;
	}
#endif

	printf("Initializing DDS!\n");
	dds_bbspi_dev dds_device;
	dds_bbspi_init(&dds_device, port);
	set_bit(&dds_device, LED_STATUSB_PIN, 1);
	set_bit(&dds_device, LED_STATUSG_PIN, 1);

	printf("Setting up DDS\n");

	// Configure bit bang device
	send_dds_configuration(&dds_device, enabled_channels);

	// Setup Single Tone Mode Profile 0 for all Channels Then Send
	for (int i = 0; i < 6; i++) {
		// Load the ASF (Amplitude Scaling Factor), POW (Phase Offset Word),
		// Frequency (in Hz)) to the channel's messages section.
		load_profile_to_channel(&dds_device, 0x3fff, 0x0, 10E6, i);
	}
	// Send to profile 0 (only on enabled_channels)
	send_dds_profile(&dds_device, enabled_channels);

	// Strobe IOUPDATE PIN to push data into active registers on AD9910
	dds_bbspi_strobe_bit(&dds_device, DDS_IOUPDATE_PIN);

	set_bit(&dds_device, LED_STATUSB_PIN, 0);

#ifndef NONARM_TEST
	//Unmap the memory and close /dev/memory
	if (munmap(port, MAP_SIZE) == -1) {
		printf("Unable to unmap port.\n");
		close(fd);
		return -1;
	}
	close(fd);
#endif

	return 0;
}
