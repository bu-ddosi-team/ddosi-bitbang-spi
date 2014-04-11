/** 
 * bitbang-spi.c : bitbang spi interface for AD9910
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: April 6, 2014
 * Team 19 dDOSI
 * Boston Unversity ECE Department Senior Design
 * Boston University Electronics Design Facility.
 */
#include "ddosi/ddosi-constants.h"
#include "ddosi/bitbang-spi.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

void set_bit(void *port, int pin, int value) {
	if (value)
		*(volatile int *)(port) |= (volatile int)((1<<pin));
	else
		*(volatile int *)(port) &= (volatile int)(~(1<<pin));
//	*(volatile int *)(port) = (volatile int)(1<<16);
//		*(volatile int *)(port) = (volatile int)(-1);
}

void dds_bbspi_idle( dds_bbspi_dev *dev) 
{
	set_bit(dev->port, DDS_IORESET_PIN, 0);
	set_bit(dev->port, DDS_IOUPDATE_PIN, 0);
	set_bit(dev->port, DDS_SCLK_PIN, 0);
	for (int i = 0; i < 6; i++) {
		set_bit(dev->port, DDS_CS_PIN(i), 1);
		set_bit(dev->port, DDS_SDO_PIN(i), 0);
	}
}

void dds_bbspi_delay( dds_bbspi_dev *dev) 
{
	nanosleep(&(dev->delay_interval_ts),NULL);
}

void dds_bbspi_init( dds_bbspi_dev *dev, void *port)
{
	// Disable all channels
	dev->ch_enable = 0;

	// Initialize instruction to a known state
	dev->instruction = 0;

	// Initialize delay interval to desired delay interval
	dev->delay_interval_ts.tv_sec = 0;
	dev->delay_interval_ts.tv_nsec = 500000L;

	// Iniitialize messages to a known state
	memset(dev->messages, 0, sizeof(dev->messages));

	// Make the devices port point to the right port
	dev->port = port;

	// Put spi bus in its idle state
	dds_bbspi_idle(dev);

	// Strobe IORESET
	dds_bbspi_strobe_bit(dev, DDS_IORESET_PIN);
}

void dds_bbspi_strobe_bit(dds_bbspi_dev *dev, int pin) 
{
		set_bit(dev->port, pin, 0);
		dds_bbspi_delay(dev);
		set_bit(dev->port, pin, 1);
		dds_bbspi_delay(dev);
		set_bit(dev->port, pin, 0);
}

void dds_bbspi_write( dds_bbspi_dev *dev ) 
{
	// Calculate message length. Depends on the DDS_EXTENDED_MESSAGE bit
	int msg_len = ((dev->ch_enable)&DDS_EXTENDED_MESSAGE)?64:32;

  // write spi message accross the whole 6 enabled channels.
	set_bit(dev->port, DDS_SCLK_PIN, 0);

	// Setup 1: Pull Chip Select Low
	for (int i = 0; i < 6; i++) {
		if ((dev->ch_enable) & (1<<i)) {
			set_bit(dev->port, DDS_CS_PIN(i), 0);	
		}
	}
	dds_bbspi_delay(dev);

	// write instruction
	dds_bbspi_shiftout_instruction(dev);

	dds_bbspi_delay(dev);

	// Writeout message
	dds_bbspi_shiftout_messages(dev, msg_len);
	dds_bbspi_delay(dev);

	// Return bus to idle state
	dds_bbspi_idle(dev);

	// Strobe IOUPDATE PIN to push data into active registers on AD9910
	dds_bbspi_strobe_bit(dev, DDS_IOUPDATE_PIN);
}


void dds_bbspi_shiftout_instruction(dds_bbspi_dev *dev)
{
	for (int i = 7; i >= 0; i--) {
		for (int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev->port, DDS_SDO_PIN(ch), (dev->instruction>>i) & 1);
			}
		}
		dds_bbspi_strobe_bit(dev, DDS_SCLK_PIN);
	}
}

void dds_bbspi_shiftout_messages(dds_bbspi_dev *dev, int msg_len)
{
	for (int i = (msg_len-1); i >= 0; i--) {
		for (int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev->port, DDS_SDO_PIN(ch), ((dev->messages[ch])>>i) & 1);
			}
		}
		dds_bbspi_strobe_bit(dev, DDS_SCLK_PIN);		
	}
}

uint64_t dds_form_profile(uint64_t asf, uint64_t pow, uint64_t ftw)
{
	return ((asf&14)<<48)|((pow&16)<<32)|(ftw);
}

uint32_t frequency2ftw(float f, float fs) {
	return (uint32_t) 0xffffffff * (2*f/fs);
}
