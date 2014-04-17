/** 
 * ddosi-constants.h : Constants for DDOSI project
 * Author: Christopher Woodall <cwoodall@bu.edu>
 * Date: April 6, 2014
 * Team 19 dDOSI
 * Boston Unversity ECE Department Senior Design
 * Boston University Electronics Design Facility.
 */
#define DDOSI_DSAU_REVA

#define GPIO0_PORT_ADDR 0x81210000
// Define Constants for DDS Controls, n represents channel.
#define DDS_CS_OFFSET (3)
#define DDS_CS_PIN(n) (DDS_CS_OFFSET + n)
#define DDS_CS0_PIN   (3)
#define DDS_CS1_PIN   (4)
#define DDS_CS2_PIN   (5)
#define DDS_CS3_PIN   (6)
#define DDS_CS4_PIN   (7)
#define DDS_CS5_PIN   (8)


#define DDS_SDO_OFFSET (9)
#define DDS_SDO_PIN(n) (DDS_SDO_OFFSET + n)
#define DDS_SDO0_PIN   (9)
#define DDS_SDO1_PIN   (10)
#define DDS_SDO2_PIN   (11)
#define DDS_SDO3_PIN   (12)
#define DDS_SDO4_PIN   (13)
#define DDS_SDO5_PIN   (14)

#define DDS_SCLK_PIN 0
#define DDS_IOUPDATE_PIN 1
#define DDS_IORESET_PIN 2

// Define LED constants (pin mappings)
#define LED_OFFSET 15
#define LED_RGB_OFFSET 19
#define LED_PIN(n) (LED_OFFSET+n)

#define LED_STATUS0_PIN 15
#define LED_STATUS1_PIN 16
#define LED_STATUS2_PIN 17
#define LED_STATUS3_PIN 18
#define LED_STATUSR_PIN 19
#define LED_STATUSG_PIN 20
#define LED_STATUSB_PIN 21
