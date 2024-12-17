// DS3231.c, DS3231 library - minimal "C" functionality by Jim Merkle
//
// Beside "Normal" time keeping functionality, the DS3231 implements:
// * Battery Backup support using a coin cell
// * Temperature compensated oscillator (increase precision over temperature range)
// * 2.3V to 5.5V operation
// * 32KHz output pin (This pin is shared with INT function)
// * +/- 2 minutes per year accuracy from -40°C to +85°C
// * Temperature register provides 1/4 degree C increments
// * Oscillator Stop Flag (OSF) indicates the oscillator has stopped (Software should clear this at power-on)
// * Two alarm registers, each of which can generate an interrupt on time match.

#include "main.h" // HAL error definitions
#include "DS3231.h"
#include <stdio.h> // printf()
#include "command_line.h"

/*====================================================================================================
| DS3231 Index Registers (See DS3231.pdf, Figure 1, Timekeeping Registers)
|=====================================================================================================
| INDEX | BIT 7 | BIT 6 | BIT 5 | BIT 4  | BIT 3 | BIT 2 | BIT 1 | BIT 0 | FUNCTION    |   RANGE     |
|  00h  |   0   |      10 Seconds        |            Seconds            |  Seconds    |   00-59     |
|  01h  |   0   |      10 Minutes        |            Minutes            |  Minutes    |   00-59     |
|  02h  |   0   | 12/24 | PM/AM |10 Hour |              Hour             |  Hours      |1-12 + PM/AM |
|       |       |       |20 Hour|        |                               |             |   00-23     |
|  03h  |   0   |   0   |   0   |   0    |   0   |      Day              | Day of Week |1-7 User Def |
|  04h  |   0   |   0   |    10 Date     |              Date             |  Date       |   01-31     |
|  05h  |Century|   0   |   0   |10 Month|             Month             |Month/Century|01-12+Century|
|  06h  |           10 Year              |              Year             |  Year       |   00-99     |
|  07h  | A1M1  |      10 Seconds        |            Seconds            |Alarm 1 Sec  |   00-59     |
|  08h  | A1M2  |      10 Minutes        |            Minutes            |Alarm 1 Min  |   00-59     |
|  09h  | A1M3  | 12/24 | PM/AM |10 Hour |              Hour             |Alarm 1 Hours|1-12 + PM/AM |
|       |       |       |20 Hour|        |                               |             |             |
|  0Ah  | A1M4  | DY/DT |    10 Date     |              Day              |Alarm 1 Day  |    1-7      |
|       |       |       |                |              Date             |Alarm 1 Date |    1-31     |
|  0Bh  | A2M2  |      10 Minutes        |            Minutes            |Alarm 2 Min  |   00-59     |
|  0Ch  | A2M3  | 12/24 | PM/AM |10 Hour |              Hour             |Alarm 2 Hours|1-12 + AM/PM |
|       |       |       |20 Hour|        |                               |             |   00-23     |
|  0Dh  | A2M4  | DY/DT |    10 Date     |              Day              |Alarm 2 Day  |    1-7      |
|       |       |       |                |              Date             |Alarm 2 Date |    1-31     |
|  0Eh  | EOSC  | BBSQW | CONV  |  RS2   |  RS1  | INTCN |  A2IE | A1IE  |  Control    |     —       |
|  0Fh  |  OSF  |   0   |   0   |   0    |EN32kHz|  BSY  |  A2F  | A1F   |Contrl/Status|     —       |
|  10h  | SIGN  | DATA  | DATA  | DATA   | DATA  | DATA  | DATA  | DATA  |Aging Offset |     —       |
|  11h  | SIGN  | DATA  | DATA  | DATA   | DATA  | DATA  | DATA  | DATA  |MSB of Temp  |     —       |
|  12h  | DATA  | DATA  |   0   |   0    |   0   |   0   |   0   |   0   |LSB of Temp  |     —       |
|====================================================================================================*/
// Notes:
// The numeric values stored to / read from the DS3231 for seconds, minutes, hours, day, date, month, year,
// use BCD encoding.

extern I2C_HandleTypeDef hi2c1; // main.c

// DS3231 registers use BCD encoding for time/date storage.
// This requires BCD to BIN and BIN to BCD functions to convert back and forth
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }

#if 0
//=============================================================================
// Initialize the DS3231, clear the OSF bit, enable the device to begin counting
//=============================================================================

// Values to write to DS3231 for a "reset" condition
const DATE_TIME dt_reset = {
	0,  //< Year offset from 2000
	1,  //< Month 1-12
	1,	//< Day 1-31
	0,  //< Hours 0-23
	0,  //< Minutes 0-59
	0   //< Seconds 0-59
};

// Write the DS3231 Control register, get RTC counting
// Does NOT clear Oscillator Stop Flag (OSF)
// Set time to reset value if clock was stopped
HAL_StatusTypeDef init_ds3231(void)
{
	uint8_t indx_control[2] = {0x0E, 0b00000000};  // Index, Control
	HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit(&hi2c1, DS3231_ADDRESS << 1, indx_control, sizeof(indx_control), HAL_I2C_SMALL_TIMEOUT);
	if(HAL_OK != rc) return rc; // if not success, return now
	// Read status register - is OSF set?
	uint8_t index = 0x0F;
	uint8_t reg_data;
	rc = i2c_write_read(DS3231_ADDRESS, &index, sizeof(index), &reg_data, sizeof(reg_data));
	if(HAL_OK != rc) return rc; // if not success, return now
	// If OSF (BIT 7) set, write initial values to RTC registers
	if(reg_data & 0x80) {
		rc = write_ds3231(&dt_reset);
	}
	return rc;
}
#endif

//=============================================================================

// Return the value of the status register (index 0Fh)
uint8_t ds3231_read_status(void)
{
  // Read value of status register into reg_data
	uint8_t index = 0x0F;
	uint8_t reg_data;
	i2c_write_read(DS3231_ADDRESS, &index, sizeof(index), &reg_data, sizeof(reg_data));
  return reg_data;
}

//=============================================================================

void ds3231_clearOSF(void)
{
	// Clear the status register (index 0Fh) OSF bit
  printf("Clear OSF\n");
	uint8_t index_status[2] = {0x0F, 0x00}; // index of status register and value to write to it
	i2c_write_read(DS3231_ADDRESS, index_status, sizeof(index_status), NULL, 0);
}

//=============================================================================

// The Time/Date registers are located at index 00h - 06h
// Use a "generic I2C API" to read index registers 000h - 06h
// Read the DS3231 time/date registers into a DATE_TIME structure
HAL_StatusTypeDef read_ds3231(DATE_TIME * dt)
{
	uint8_t index = 0;
	uint8_t reg_data[7];
	HAL_StatusTypeDef rc = i2c_write_read(DS3231_ADDRESS, &index, sizeof(index), reg_data, sizeof(reg_data));
	if(HAL_OK != rc) return rc; // if not success, return now
	// Convert DS3231 register data into DATE_TIME format
	dt->ss = bcd2bin(reg_data[0]);
	dt->mm = bcd2bin(reg_data[1]);
	dt->hh = bcd2bin(reg_data[2]); // bit 6 should be low (We can force it...)
	dt->d  = bcd2bin(reg_data[4]);
	dt->m  = bcd2bin(reg_data[5] & 0x1F); // remove century bit
	dt->yOff = bcd2bin(reg_data[6]);
	return rc;
}

//=============================================================================

// The Time/Date registers are located at index 00h - 06h
// Use a "generic I2C API" to write index registers 000h - 06h
// Write the DS3231 time/date registers given a DATE_TIME structure
HAL_StatusTypeDef write_ds3231(const DATE_TIME * dt)
{
	uint8_t reg_data[8]; // [0] index, [1] - [7] time and date registers

	// Convert DATE_TIME format into DS3231 register values
	reg_data[0] = 0x00; // begin writing at index 0
	reg_data[1] = bin2bcd(dt->ss);
	reg_data[2] = bin2bcd(dt->mm);
	reg_data[3] = bin2bcd(dt->hh); // bit 6 should be low (We can force it...)
	reg_data[4] = 0; // day of the week (don't care)
	reg_data[5] = bin2bcd(dt->d);
	reg_data[6] = bin2bcd(dt->m); // remove century bit
	reg_data[7] = bin2bcd(dt->yOff);

	HAL_StatusTypeDef rc = i2c_write_read(DS3231_ADDRESS, reg_data, sizeof(reg_data), NULL, 0);
	if(HAL_OK != rc) return rc; // if not success, return now

	// Clear the OSF bit
	uint8_t index_status[2] = {0x0F, 0x00}; // index of status register and value to write to it
	rc = i2c_write_read(DS3231_ADDRESS, index_status, sizeof(index_status), NULL, 0);
	return rc;
}

// Command line method to read / set the time
int cl_time(void)
{
	DATE_TIME dt;
	read_ds3231(&dt); // read in time and date values into DATE_TIME structure

	if(4 == argc) {
		// Set the time using arguments at index 1 <hours>, 2 <minutes>, 3 <seconds>
		dt.hh = strtol(argv[1], NULL, 10); // user will use decimal
		dt.mm = strtol(argv[2], NULL, 10);
		dt.ss = strtol(argv[3], NULL, 10);
		// Write new time values to DS3231
		write_ds3231(&dt);
	    // Reset the OSF bit
	    ds3231_clearOSF();
	}

	// Always read the DS3231 and display the time
	read_ds3231(&dt);
	printf("%02u:%02u:%02u\r\n",dt.hh,dt.mm,dt.ss);
	return 0;
}

// Command line method to read / set the date
int cl_date(void)
{
	DATE_TIME dt;
	read_ds3231(&dt); // read in time and date values into DATE_TIME structure

	if(4 == argc) {
		// Set the date using arguments at index 1 <day>, 2 <month>, 3 <year>
		dt.d = strtol(argv[1], NULL, 10); // user will use decimal
		dt.m = strtol(argv[2], NULL, 10);
		uint16_t year = strtol(argv[3], NULL, 10);
		if(year >= 2000) year -= 2000; // convert to offset
		dt.yOff = (uint8_t)year;
		// Write new date values to DS3231
		write_ds3231(&dt);
	}

	// Always read the DS3231 and display the date
	read_ds3231(&dt);
	printf("%02u/%02u/%04u\r\n",dt.d,dt.m,dt.yOff + 2000);
	return 0;
}


// Dump the values of the 19 DS3231 index registers 00h - 12h
// Use a "generic I2C API" to read index registers 00h - 12h
// Read the DS3231 time/date registers into a DATE_TIME structure
int cl_ds3231_dump(void)
{
    uint8_t index = 0;
    uint8_t reg_data[19];
    const char * reg_name[]={"Seconds","Minutes","Hours","WeekDay","Date","Month","Year",
        "Alarm1 Sec","Alarm1 Min","Alarm1 Hr","Alarm1 Day-Date",
        "Alarm2 Min","Alarm2 Hr","Alarm2 Day-Date",
        "Control","Cntrl/Status","Aging Offset","MSB of Temp","LSB of Temp"};
    i2c_write_read(DS3231_ADDRESS, &index, sizeof(index), reg_data, sizeof(reg_data));
    printf("Indx Data   Register name\n");
    for(unsigned i=0;i<19;i++) {
        printf("%02X   0x%02X   %s\n",i,reg_data[i],reg_name[i]);
    }
	printf("\n");
	return 0;
}


// Test the INT/SQW output
// Allow the user to enable one of several test functions via an argument
// 0: 1Hz, 1: 1024Hz, 2: 4096Hz, 3:8192Hz
int cl_sqw_test(void)
{
  if(argc > 1) {
    uint8_t RSx = (uint8_t) strtol(argv[1], NULL, 10); // assume decimal input
    RSx &= 3; // keep only lower 2 bits
    printf("%s: input: %u\n",__func__,RSx);
    RSx <<=3; // move the 2 bits into BIT4:BIT3 position (The value of RSx, written to Control Register (0Eh), will now enable SQW signal)
  	uint8_t index_data[2] = {0x0E,0x1C}; // write default (POR) value, turning off SQW output
    i2c_write_read(DS3231_ADDRESS, index_data, sizeof(index_data), NULL, 0);
    index_data[1] = RSx; // desired register - index 0Eh value
    i2c_write_read(DS3231_ADDRESS, index_data, sizeof(index_data), NULL, 0);
    printf("0X%02X written to index 0Eh\n",RSx);
  }
  else {
    // no arguments
#if 0
    DATE_TIME dt;
    printf("Setting alarm for 3 seconds from now...\n");
    // Get current time and convert it to unix seconds count
    read_ds3231(&dt); // fill in DATE_TIME structure
    uint32_t ut_now = rtc2unix(&dt);
    uint32_t ut_future = ut_now + 3;
    // Convert back into DATE-TIME format
    unix2rtc(&dt, ut_future); // DATE_TIME is now 3 seconds in the future
#else
    printf("%s: Turn off SQW output\n",__func__);
    uint8_t index_data[2] = {0x0E,0x1C}; // write default (POR) value, turning off SQW output
    i2c_write_read(DS3231_ADDRESS, index_data, sizeof(index_data), NULL, 0);
#endif
  }

  return 0;
}

