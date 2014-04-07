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

void set_bit(volatile int *port, int pin, int value) {
	if (value) {
		*port |= (1<<pin);
	} else {
		*port &= ~(1<<pin);
	}
}

void dds_bbspi_disable( dds_bbspi_dev *dev) 
{
	set_bit(dev->port, DDS_IORESET_PIN, 0);
	set_bit(dev->port, DDS_IOUPDATE_PIN, 0);
	set_bit(dev->port, DDS_SCLK_PIN, 0);
	for (int i = 0; i < 6; i++) {
		set_bit(dev->port, DDS_CS_PIN(i), 1);
		set_bit(dev->port, DDS_SDO_PIN(i), 0);
	}
}

void dds_bbspi_init( dds_bbspi_dev *dev, volatile int *port)
{
	dev->ch_enable = 0;
	dev->instruction = 0;
	dev->delay_interval_ts.tv_sec = 0;
	dev->delay_interval_ts.tv_nsec = 500000L;
	for (int i = 0; i < 6; i++) {
		dev->messages[i] = 0;
	}

	dev->port = port;
	dds_bbspi_disable(dev);

	set_bit(dev->port, DDS_IORESET_PIN, 1);
	nanosleep(&(dev->delay_interval_ts),NULL);
	set_bit(dev->port, DDS_IORESET_PIN, 0);
}

void dds_bbspi_write( dds_bbspi_dev *dev ) 
{
	int msg_len = ((dev->ch_enable)&DDS_EXTENDED_MESSAGE)?64:32;

  // write spi message accross the whole 6 enabled channels.
	printf("\n%08x\n", *(dev->port));
	// Setup 1: Pull Chip Select Low
	for (int i = 0; i < 6; i++) {
		if ((dev->ch_enable) & (1<<i)) {
			set_bit(dev->port, DDS_CS_PIN(i), 0);	
			nanosleep(&(dev->delay_interval_ts),NULL);
		}
	}
	

	for (int i = 7; i >= 0; i--) {
		set_bit(dev->port, DDS_SCLK_PIN, 0);
		for (int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev->port, DDS_SDO_PIN(ch), (dev->instruction>>i) & 1);
			}
		}
		nanosleep(&(dev->delay_interval_ts),NULL);
		
		set_bit(dev->port, DDS_SCLK_PIN, 1);
		nanosleep(&(dev->delay_interval_ts),NULL);
		
	}
	
	for (int i = (msg_len-1); i >= 0; i--) {
		
		set_bit(dev->port, DDS_SCLK_PIN, 0);
		for (int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev->port, DDS_SDO_PIN(ch), ((dev->messages[ch])>>i) & 1);
			}
		}
		nanosleep(&(dev->delay_interval_ts),NULL);
		
		set_bit(dev->port, DDS_SCLK_PIN, 1);		
		nanosleep(&(dev->delay_interval_ts),NULL);
		
	}
	set_bit(dev->port, DDS_SCLK_PIN, 0);
	nanosleep(&(dev->delay_interval_ts),NULL);
	dds_bbspi_disable(dev);
	
	// Strobe IOUPDATE PIN to push data into active registers on AD9910
	nanosleep(&(dev->delay_interval_ts),NULL);
	set_bit(dev->port, DDS_IOUPDATE_PIN, 1);
	nanosleep(&(dev->delay_interval_ts),NULL);
	set_bit(dev->port, DDS_IOUPDATE_PIN, 0);
	nanosleep(&(dev->delay_interval_ts),NULL);
	
}
