/** 
 * ddosi-constants.h : Constants for DDOSI project
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: April 6, 2014
 * Team 19 dDOSI
 * Boston Unversity ECE Department Senior Design
 * Boston University Electronics Design Facility.
 */
#define DDOSI_DSAU_REVA

#define GPIO0_ADDRESS (0x4200) // FIXME

// Define Constants for DDS Controls, n represents channel.
#define DDS_CS_OFFSET (3)
#define DDS_CS_PIN(n) (DDS_CS_OFFSET + n)

#define DDS_SDO_OFFSET (9)
#define DDS_SDO_PIN(n) (DDS_SDO_OFFSET + n)

#define DDS_SCLK_PIN 0
#define DDS_IOUPDATE_PIN 1
#define DDS_IORESET_PIN 2

// Define LED constants (pin mappings
#define LED_OFFSET 15
#define LED_PIN(n) (LED_OFFSET+n)
//#define ASM_NOP (asm("mov r0,r0"))
