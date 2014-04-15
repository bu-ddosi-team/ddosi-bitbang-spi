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

void set_bit(dds_bbspi_dev *dev, unsigned int pin, uint64_t value) {
	if (value) {
		dev->port |= (1<<pin);
//		printf("1");
	} else {
		dev->port &= ~(1<<pin);
//		printf("0");
	}


	*(volatile unsigned int *)(dev->_port) = dev->port;
}

void dds_bbspi_idle( dds_bbspi_dev *dev) 
{
	set_bit(dev, DDS_IORESET_PIN, 0);
	set_bit(dev, DDS_IOUPDATE_PIN, 0);
	set_bit(dev, DDS_SCLK_PIN, 0);
	for (unsigned int i = 0; i < 6; i++) {
		set_bit(dev, DDS_CS_PIN(i), 1);
		set_bit(dev, DDS_SDO_PIN(i), 0);
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

	// Initialize messages to a known state (0s)
	memset(dev->messages, 0, sizeof(dev->messages));

	// Make the devices port point to the right port
	dev->_port = port;
	dev->port = 0;

	// Put spi bus in its idle state
	dds_bbspi_idle(dev);

	// Strobe IORESET
	dds_bbspi_strobe_bit(dev, DDS_IORESET_PIN);
}

void dds_bbspi_strobe_bit(dds_bbspi_dev *dev, unsigned int pin) 
{
		set_bit(dev, pin, 0);
		dds_bbspi_delay(dev);
		set_bit(dev, pin, 1);
		dds_bbspi_delay(dev);
		set_bit(dev, pin, 0);
}

void dds_bbspi_write( dds_bbspi_dev *dev ) 
{
	// Calculate message length. Depends on the DDS_EXTENDED_MESSAGE bit
	int msg_len = ((dev->ch_enable)&DDS_EXTENDED_MESSAGE)?64:32;

	dds_bbspi_idle(dev);
  // write spi message accross the whole 6 enabled channels.
	set_bit(dev, DDS_SCLK_PIN, 0);

	// Setup 1: Pull Chip Select Low
	for (unsigned int i = 0; i < 6; i++) {
		if ((dev->ch_enable) & (1<<i)) {
			set_bit(dev, DDS_CS_PIN(i), 0);	
		}
	}
	dds_bbspi_delay(dev);

	// write instruction
	dds_bbspi_shiftout_instruction(dev);
	printf("Instruction\n");

	dds_bbspi_delay(dev);
	dds_bbspi_delay(dev);
	dds_bbspi_delay(dev);

	// Writeout message
	dds_bbspi_shiftout_messages(dev, msg_len);
	printf("Messages\n");
	dds_bbspi_delay(dev);
	printf("Done\n");
	// Return bus to idle state
	dds_bbspi_idle(dev);
}


void dds_bbspi_shiftout_instruction(dds_bbspi_dev *dev)
{
	for (unsigned int i = 0; i < 8; i++) {
		for (unsigned int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev, DDS_SDO_PIN(ch), (dev->instruction<<i) & 0x80);
			}
		}
		dds_bbspi_strobe_bit(dev, DDS_SCLK_PIN);
	}
}

void dds_bbspi_shiftout_messages(dds_bbspi_dev *dev, unsigned int msg_len)
{
	for (unsigned int i = msg_len; i > 0; i--) {
		for (unsigned int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev, DDS_SDO_PIN(ch), (dev->messages[ch])>>(i-1) & 1);
//				printf("%i", (int)((dev->messages[ch])>>(i-1) & 1));
			}
		}
		dds_bbspi_strobe_bit(dev, DDS_SCLK_PIN);		
	}
}

uint64_t dds_form_profile(uint64_t asf, uint64_t pow, uint64_t ftw)
{
	
	return ((asf&0x3FFF)<<48)|((pow&0xFFFF)<<32)|(ftw);
}

uint32_t frequency2ftw(float f, float fs) {
	return (uint32_t) ((float)(4294967296.0)*((float)f/(float)fs));
}
