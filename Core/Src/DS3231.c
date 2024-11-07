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

// DS3231 registers use BCD encoding for time/date storage.
// This requires BCD to BIN and BIN to BCD functions to convert back and forth
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }

//=============================================================================
// Initialize the DS3231, clear the OSF bit, enable the device to begin counting
// Configure Control register and Status register as follows:
//      Control Register (0Eh)
//      +-------+-------+-------+-------+-------+-------+-------+-------+
// BIT  | BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
// NAME | /EOSC | BBSQW | CONV  |  RS2  |  RS1  | INTCN | A2IE  | A1IE  |
// POR  |   0   |   0   |   0   |   1   |   1   |   1   |   0   |   0   |
//      +-------+-------+-------+-------+-------+-------+-------+-------+
//      Status Register (0Fh)
//      +-------+-------+-------+-------+-------+-------+-------+-------+
// BIT  | BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
// NAME |  OSF  |       |       |       |EN32KHz|  BSY  |  A2F  |  A1F  |
// POR  |   1   |   0   |   0   |   0   |   1   |   X   |   X   |   X   |
//      +-------+-------+-------+-------+-------+-------+-------+-------+

extern I2C_HandleTypeDef hi2c1; // main.c

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
//
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
		//printf("Writing time values - -  %02u:%02u:%02u\r\n",dt.hh,dt.mm,dt.ss);
		printf("Writing time values...\r\n");
		write_ds3231(&dt);
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
		// Write new time values to DS3231
		//printf("Writing date values - -  %02u/%02u/%04u\r\n",dt.d,dt.m,dt.yOff+2000);
		printf("Writing date values...\r\n");
		write_ds3231(&dt);
	}

	// Always read the DS3231 and display the date
	read_ds3231(&dt);
	printf("%02u/%02u/%04u\r\n",dt.d,dt.m,dt.yOff + 2000);
	return 0;
}
