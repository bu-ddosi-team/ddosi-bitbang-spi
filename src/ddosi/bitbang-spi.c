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

/**
 * Set or unset bit depending on value. 
 *
 * @param  dev    pointer to dds_bbspi_dev whose port is having a bit set
 * @param  pin    which bit in the port is being set or unset. 
                  bit counting stats from 0.
 * @param  value  if '0' unset bit, if non-zero set bit.
 */
void set_bit(dds_bbspi_dev *dev, unsigned int pin, unsigned int value) {
	// port is a local copy of the port
	if (value) {
		dev->port |= (1<<pin);
	} else {
		dev->port &= ~(1<<pin);
	}

	// update actual port to match local copy of port
	*(volatile unsigned int *)(dev->_port) = dev->port;
}

/**
 * Set outputs of the port "dev" refers to an idle state. Only effects DDS related
 * pins.
 *
 * @param  dev  pointer to dds_bbspi_dev whose port is being set to an idle state.
 */
void dds_bbspi_idle( dds_bbspi_dev *dev) 
{
	// Note: This is really an inefficient way to set the port, but effective
	//       and easy to work with.
	set_bit(dev, DDS_IORESET_PIN, 0);  // Idle off
	set_bit(dev, DDS_IOUPDATE_PIN, 0); // Idle off
	set_bit(dev, DDS_SCLK_PIN, 0);     // Idle off (rising-edge triggered)

	// Set bits which are shared accross all 6 channels
	for (unsigned int i = 0; i < 6; i++) {
		set_bit(dev, DDS_CS_PIN(i), 1);  // Active Low
		set_bit(dev, DDS_SDO_PIN(i), 0); // Do Not care
	}
}

/**
 * Uses nanosleep and a timespec stored in dev to delay. Technically has
 * "nanosecond" precision. works well at about 50us... Likely will work well
 * at 10us (likely breaks in consistency around here). Performance should
 * depend on computational load.
 *
 * @param  dev  pointer to dds_bbspi_dev which has a sleep timer setup
 */
void dds_bbspi_delay( dds_bbspi_dev *dev) 
{
	nanosleep(&(dev->delay_interval_ts),NULL);
}

/**
 * Initializes a dds_bbspi_dev struct to a known state. Also,
 * "binds" a port to the device.
 *
 * @param  dev   pointer to the device to initialize
 * @param  port  Pointer to the port to refer to in dev
 */
void dds_bbspi_init( dds_bbspi_dev *dev, void *port)
{
	// Disable all channels
	dev->ch_enable = 0;

	// Initialize instruction to a known state
	dev->instruction = 0;

	// Initialize delay interval to desired delay interval
	dev->delay_interval_ts.tv_sec = 0;
	dev->delay_interval_ts.tv_nsec = 50000L;

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

/**
 * Turn a bit in a port on and off with the delay specified in dev in between. Only capeable of 0 -> 1 -> 0 strobes.
 *
 * @param  dev  device which is being effected
 * @param  pin  which bit on the port to strobe
 */
void dds_bbspi_strobe_bit(dds_bbspi_dev *dev, unsigned int pin) 
{
		set_bit(dev, pin, 0);
		dds_bbspi_delay(dev);
		set_bit(dev, pin, 1);
		dds_bbspi_delay(dev);
		set_bit(dev, pin, 0);
}

/**
 * Write to the SPI bus. Writes instruction to all ports then contents of messages[0..5] uint64_t array.
 *
 * If an extended write is desired it must be toggled in the ch_enable member of the dds_bbspi_dev struct being
 * passed to dds_bbspi_write.
 *
 * @param  dev  device which is being written to. Device also contains message and instructions
 */
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

/**
 * Send out instruction in dev to all channels
 */
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

/**
 * Send out messages in dev to all channels
 */
void dds_bbspi_shiftout_messages(dds_bbspi_dev *dev, unsigned int msg_len)
{
	for (unsigned int i = msg_len; i > 0; i--) {
		for (unsigned int ch = 0; ch < 6; ch++) {
			if ((dev->ch_enable) & (1<<ch)) {
				set_bit(dev, DDS_SDO_PIN(ch), (dev->messages[ch])>>(i-1) & 1);
			}
		}
		dds_bbspi_strobe_bit(dev, DDS_SCLK_PIN);		
	}
}

/**
 * Take asf, pow and ftw as arguments and return a correctly formatted profile. 
 *
 * @param asf  14-bit amplitude scale factor.
 * @param pow  16-bit phase offset.
 * @param ftw  32-bit Frequency Tuning Word.
 */
uint64_t dds_form_profile(uint64_t asf, uint64_t pow, uint64_t ftw)
{
		return ((asf&0x3FFF)<<48)|((pow&0xFFFF)<<32)|(ftw);
}

/**
 * Take a frequency and a sampling frequency and convert it to a 32 bit number for
 * use as a frequency tuning word.
 *
 * @param f   Frequency we want to represent as ftw (32 bit binary number)
 * @param fs  "Sampling" frequency of DDS
 */
uint32_t frequency2ftw(float f, float fs) {
	// Formula taken from datasheet of AD9910
	return (uint32_t) ((float)(4294967296.0)*((float)f/(float)fs));
}

/**
 * Send  standard working dds_configuration.
 */
void send_dds_configuration(dds_bbspi_dev *dds_device, unsigned int enabled_channels) {
	// Setup standard channel configurations
	unsigned int cfr1_settings = (1<<1); // Set SDIO to input only

	unsigned int cfr2_settings = (1<<24) | 
                               (1<<22) |
                               (1<<5); // Enable Amplitude Scaling

	// Configures REFCLK_OUT (PLL config) FIXME
	unsigned int cfr3_settings = (0x3 << 28) | // High output current on refclk_out
	                             (0x5 << 24) | // Setup VCO to VCO3
		                           (0x7 << 19) | // PLL Charge Pump Current (FIXME)
		                           (1 << 15)   | // Bypass ref_clk divider
		                           (1 << 14)   | // Reset Divider acts normally
		                           (1 << 8)    | // Enable PLL
                               (40 << 1);    // Multiplication factor of 20 
                                             // (40*25MHz = 1000MHz)

	// Setup CFR1
	dds_device->ch_enable = enabled_channels;
	dds_device->instruction = DDS_WRITE | DDS_CFR1;

	for (int i = 0; i < 6; i++) {
		dds_device->messages[i] = cfr1_settings;
	}

	dds_bbspi_write(dds_device);

	dds_bbspi_delay(dds_device);
	dds_bbspi_delay(dds_device);

	// Setup CFR2
	dds_device->ch_enable = enabled_channels;
	dds_device->instruction = DDS_WRITE | DDS_CFR2;
	for (int i = 0; i < 6; i++) {
		dds_device->messages[i] = cfr2_settings;
	}
	dds_bbspi_write(dds_device);

	dds_bbspi_delay(dds_device);
	dds_bbspi_delay(dds_device);

	// Setup CFR3
	dds_device->ch_enable = enabled_channels;
	dds_device->instruction = DDS_WRITE | DDS_CFR3;
	for (int i = 0; i < 6; i++) {
		dds_device->messages[i] = cfr3_settings;
	}
	dds_bbspi_write(dds_device);

	dds_bbspi_delay(dds_device);
	dds_bbspi_delay(dds_device);
}

/**
 * Wrapper for sending a profile. takes enabled channels, ensures that ch_enable is set write. Also sets up the write instruction.
 */
void send_dds_profile(dds_bbspi_dev *dds_device, unsigned int enabled_channels) {
	dds_device->ch_enable = enabled_channels | DDS_EXTENDED_MESSAGE;
	dds_device->instruction = DDS_WRITE | DDS_PROFILE_0;
	dds_bbspi_write(dds_device);
}

/**
 * Wrapper for loading a profile into the message array in dds_device. Which element in the array is specified by ch
 */
void load_profile_to_channel(dds_bbspi_dev *dds_device, uint32_t asf, uint64_t pow, float freq, unsigned int ch) {
	if (ch < 6) {
		dds_device->messages[ch] = dds_form_profile(asf, pow, 
																							 frequency2ftw(freq,DDS_FS));
	}
}


