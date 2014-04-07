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

int main() {
	int a;
	volatile int *port = &a;

	printf("Initializing DDS!");
	dds_bbspi_dev dds_device;

	dds_bbspi_init(&dds_device, port);
	dds_device.ch_enable = DDS_CH0 | DDS_EXTENDED_MESSAGE;
	dds_device.instruction = DDS_WRITE | DDS_CFR2;
	dds_device.messages[0] = 0xff00ff00ff00ff00;
	dds_device.messages[1] = 0xff00ff00ff00ff;
	dds_device.messages[2] = 0xff00ff00ff00ff00;
	dds_device.messages[3] = 0xff00ff00ff00ff;
	dds_device.delay_interval_ts.tv_nsec = 50000L;
	dds_bbspi_write(&dds_device);
	
	return 0;
}
