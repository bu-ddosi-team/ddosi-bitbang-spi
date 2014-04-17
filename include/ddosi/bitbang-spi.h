/** 
 * bitbang-spi.h : bitbang spi interface for AD9910
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: April 6, 2014
 * Team 19 dDOSI
 * Boston Unversity ECE Department Senior Design
 * Boston University Electronics Design Facility.
 */
#define _POSIX_C_SOURCE 199309
#include "ddosi-constants.h"
#include <stdint.h>
#include <time.h>

// Define flags for channel enabling
#define DDS_NONE 0
#define DDS_CH0 1<<0
#define DDS_CH1 1<<1
#define DDS_CH2 1<<2
#define DDS_CH3 1<<3
#define DDS_CH4 1<<4
#define DDS_CH5 1<<5
#define DDS_ALL 0x7f
#define DDS_EXTENDED_MESSAGE 0x80

// Read write bitmasks
#define DDS_READ  1<<7
#define DDS_WRITE 0

// Address to name mappings for DDS registers
#define DDS_CFR1 0x00
#define DDS_CFR2 0x01
#define DDS_CFR3 0x02
#define DDS_AUX_DAC_CTRL 0x03
#define DDS_IOUPDATE_RATE 0x04
#define DDS_FTW 0x07
#define DDS_POW 0x08
#define DDS_ASF 0x09
#define DDS_MULTI_SYNC 0x0A
#define DDS_PROFILE_0 0x0E 
#define DDS_PROFILE_1 0x0F
#define DDS_PROFILE_2 0x10
#define DDS_PROFILE_3 0x11
#define DDS_PROFILE_4 0x12
#define DDS_PROFILE_5 0x13
#define DDS_PROFILE_6 0x14
#define DDS_PROFILE_7 0x15
#define DDS_FS (1.0E9)


// Struct for maintianing information about the DDS SPI system
typedef struct {
//	volatile int *port;
	void *_port; 
	unsigned int port;
	char ch_enable; // Lower 6 bits enable channels
	                // Top bit (0x80) enables extended mode
	                // (send 64 bits instead of 32 for messages)

	char instruction; // 8 bit instruction, top bit is read/write, remaining is
	                  // is the address of the register
	uint64_t messages[6]; // message to write to the register.

	struct timespec delay_interval_ts; // delay to slow down toggles and make clock
	                                   // "more predictable"
} dds_bbspi_dev;

// helper function for setting bits based on value
void set_bit(dds_bbspi_dev *dev, unsigned int pin, unsigned int value);

// intialize dds bitbanged spi device with the port.
//void dds_bbspi_init( dds_bbspi_dev *dev , volatile int *port);
void dds_bbspi_init( dds_bbspi_dev *dev , void *port);

// turn off all chip selects and set to idle state
void dds_bbspi_idle( dds_bbspi_dev *dev);

// write out the messags on all the channels then strobe IOUPDATE.
void dds_bbspi_write( dds_bbspi_dev *dev );

// use nanosleep to delay. simple wrapper.
void dds_bbspi_delay( dds_bbspi_dev *dev);

//  Turn a bit of the port on and off for delaytime specified in dev
void dds_bbspi_strobe_bit(dds_bbspi_dev *dev, unsigned int pin);

void dds_bbspi_shiftout_instruction(dds_bbspi_dev *dev);
void dds_bbspi_shiftout_messages(dds_bbspi_dev *dev, unsigned int msg_len);

uint64_t dds_form_profile(uint64_t asf, uint64_t pow, uint64_t ftw);
uint32_t frequency2ftw(float f, float fs);

void send_dds_configuration(dds_bbspi_dev *dds_device, unsigned int enabled_channels);
void send_dds_profile(dds_bbspi_dev *dds_device, unsigned int enabled_channels);
void load_profile_to_channel(dds_bbspi_dev *dds_device, uint32_t asf, uint64_t pow, float freq, unsigned int ch);
