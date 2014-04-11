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

#define DDS_FS (1E9)

//The GPIO bus address for the DDS signals
unsigned long int PORT_ADDR=0x81210000;

//Must allocate in increments of full blocks. The block size
//for the RAM on the microZed is 4K, seems wasteful for 4 bytes,
//but that's how it goes.
#define MAP_SIZE 4096UL

int main() {
	//Open the RAM and map port to it.
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

	printf("Initializing DDS!\n");
	dds_bbspi_dev dds_device;

	// Configure bit bang device
	dds_bbspi_init(&dds_device, port);
	dds_device.delay_interval_ts.tv_nsec = 50000L;

	// Enable channels
	int enabled_channels = DDS_CH0;
	// Options we may want to allow
	//   - Inverse sinc filter enable (bit 22)
	//
	int cfr1_settings = (1<<1); // Set SDIO to input only
	int cfr2_settings = (1<<24); // Enable Amplitude Scaling

	// Configures REFCLK_OUT (PLL config) FIXME
	int cfr3_settings = (1<<28) | // Low output current on refclk_out
                   	  (1<<24) | // Setup VCO FIXME (FIXME)
		                  (1<<19)  | // PLL Charge Pump Current (FIXME?)
				  (1<<15) | // Bypass refclk divider (FIXME?)
		                  (1<<8);   // Enable PLL
	// Make a profile from different amplitude, phase and frequency settings
	int profile0 = dds_form_profile(0x3fff, 0x0, frequency2ftw(200E6, DDS_FS));

	// Setup CFR1
	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR1;
	dds_device.messages[0] = cfr1_settings;
	dds_bbspi_write(&dds_device);

	volatile int readVal; //This is the current value of port
	readVal = *(volatile unsigned int*)(port);
        printf("DDS after CFR1 = 0x%08x\n",readVal);


	// Setup CFR2
	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR2;
	dds_device.messages[0] = cfr2_settings;
	dds_bbspi_write(&dds_device);

	// Setup CFR3
	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR3;
	dds_device.messages[0] = cfr3_settings;
	dds_bbspi_write(&dds_device);

	// Set profile0
	dds_device.ch_enable = enabled_channels | DDS_EXTENDED_MESSAGE;
	dds_device.instruction = DDS_WRITE | DDS_PROFILE_0;
	dds_device.messages[0] = profile0;
	dds_bbspi_write(&dds_device);

	//Unmap the memory and close /dev/memory
	if (munmap(port, MAP_SIZE) == -1) {
		printf("Unable to unmap port.\n");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}
