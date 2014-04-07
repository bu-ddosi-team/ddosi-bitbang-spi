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

#define DDS_FS 1E9

int main() {
	int a;
	volatile int *port = &a;

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
	int cfr3_settings = (1<<28)    | // Low output current on refclk_out
                   	  (0x01<<24) | // Setup VCO FIXME (FIXME)
                   	  (0x01<<19  | // PLL Charge Pump Current (FIXME?)
										  (1<<15)    | // Bypass refclk divider (FIXME?)
		                  (1<<8);      // Enable PLL

	int profile0 = dds_form_profile(0x3fff, 0x0, frequency2ftw(200E6, DDS_FS));

	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR1;
	dds_device.messages[0] = cfr1_settings;
	dds_bbspi_write(&dds_device);

	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR2;
	dds_device.messages[0] = cfr2_settings;
	dds_bbspi_write(&dds_device);

	dds_device.ch_enable = enabled_channels;
	dds_device.instruction = DDS_WRITE | DDS_CFR3;
	dds_device.messages[0] = cfr3_settings;
	dds_bbspi_write(&dds_device);

	dds_device.ch_enable = enabled_channels | DDS_EXTENDED_MESSAGE;
	dds_device.instruction = DDS_WRITE | DDS_PROFILE_0;
	dds_device.messages[0] = profile0;
	dds_bbspi_write(&dds_device);

	
	return 0;
}
