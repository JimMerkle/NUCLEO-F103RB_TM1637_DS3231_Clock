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

// DS3231 registers use BCD encoding for time/date storage.
// This requires BCD to BIN and BIN to BCD functions to convert back and forth
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }

//=============================================================================
extern I2C_HandleTypeDef hi2c1; // using I2C1 - global instance

// Implement a "generic I2C API" for writing to and then reading from an I2C device (in that order)
// Model this to be similar to HAL I2C APIs
HAL_StatusTypeDef i2c_write_read(uint16_t DevAddress, uint8_t * write_data, uint16_t write_count, uint8_t * read_data, uint16_t read_count)
{
	HAL_StatusTypeDef rc = HAL_OK;
	// If write_data and wrire_count are non-null, perform write first
	if(write_data && write_count) {
		rc = HAL_I2C_Master_Transmit(&hi2c1, DevAddress << 1, write_data, write_count, SMALL_HAL_TIMEOUT);
		if(HAL_OK != rc) printf("HAL_I2C_Master_Transmit() Error: %d\r\n",rc);
	}

	if(HAL_OK == rc && read_data && read_count) {
		rc = HAL_I2C_Master_Receive(&hi2c1, DevAddress << 1, read_data, read_count, SMALL_HAL_TIMEOUT);
		if(HAL_OK != rc) printf("HAL_I2C_Master_Receive() Error: %d\r\n",rc);
	}

	return rc;
}
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
HAL_StatusTypeDef init_ds3231(void)
{
	uint8_t control_status[3] = {0x0E, 0b00000000, 0b00000000};  // Index, Control, Status
	return HAL_I2C_Master_Transmit(&hi2c1, DS3231_ADDRESS << 1, control_status, sizeof(control_status), SMALL_HAL_TIMEOUT);
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
HAL_StatusTypeDef write_ds3231(DATE_TIME * dt)
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

	return i2c_write_read(DS3231_ADDRESS, reg_data, sizeof(reg_data), NULL, 0);
}

