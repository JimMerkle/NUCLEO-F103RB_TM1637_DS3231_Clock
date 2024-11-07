// Copyright Jim Merkle, 2/17/2020
// File: cl_i2c.c
//
// I2C routines for the command line interface
// All the routines and command line interface assume the usage of 7-bit I2C address in the range 0x03 to 0x77.

#include <stdio.h>
#include <stdint.h> // uint8_t
#include <stdlib.h> // strtol()
#include "main.h"   // HAL functions and defines - HAL_I2C_MODULE_ENABLED in stm32f1xx_hal_conf.h
#include "cl_i2c.h"

// I2C helper function that validates I2C address is within range
// If I2C address is within range, return 0, else display error and return -1.
int cl_i2c_validate_address(uint16_t i2c_address)
{
	if(i2c_address < I2C_ADDRESS_MIN || i2c_address > I2C_ADDRESS_MAX) {
		printf("Address out of range. Expect 0x%02X to 0x%02X\n",I2C_ADDRESS_MIN,I2C_ADDRESS_MAX); // out of range
		return -1;
	}
	return 0; // success
}

extern I2C_HandleTypeDef hi2c1; // using I2C1 - global instance

// Implement a "generic I2C API" for writing to and then reading from an I2C device (in that order)
// Model this to be similar to HAL I2C APIs
HAL_StatusTypeDef i2c_write_read(uint16_t DevAddress, uint8_t * write_data, uint16_t write_count, uint8_t * read_data, uint16_t read_count)
{
	HAL_StatusTypeDef rc = HAL_OK;
	// If write_data and wrire_count are non-null, perform write first
	if(write_data && write_count) {
		rc = HAL_I2C_Master_Transmit(&hi2c1, DevAddress << 1, write_data, write_count, HAL_I2C_SMALL_TIMEOUT);
		if(HAL_OK != rc) printf("HAL_I2C_Master_Transmit() Error: %d\r\n",rc);
	}

	if(HAL_OK == rc && read_data && read_count) {
		rc = HAL_I2C_Master_Receive(&hi2c1, DevAddress << 1, read_data, read_count, HAL_I2C_SMALL_TIMEOUT);
		if(HAL_OK != rc) printf("HAL_I2C_Master_Receive() Error: %d\r\n",rc);
	}

	return rc;
}

// Perform an I2C bus scan similar to Linux's i2cdetect, or Arduino's i2c_scanner sketch
int cl_i2c_scan(void)
{
    printf("I2C Scan - scanning I2C addresses 0x%02X - 0x%02X\n",I2C_ADDRESS_MIN,I2C_ADDRESS_MAX);
    // Display Hex Header
    printf("    "); for(int i=0;i<=0x0F;i++) printf(" %0X ",i);
    // Walk through address range 0x00 - 0x77, but only test 0x03 - 0x77
    for(uint16_t addr=0;addr<=I2C_ADDRESS_MAX;addr++) {
    	// If address defines the beginning of a row, start a new row and display row text
    	if(!(addr%16)) printf("\n%02X: ",addr);
		// Check I2C addresses in the range 0x03-0x7F
		if(addr < I2C_ADDRESS_MIN || addr > I2C_ADDRESS_MAX) {
			printf("   "); // out of range
			continue;
		}
		// Perform I2C device detection - returns HAL_OK if device found
		if(HAL_OK == HAL_I2C_IsDeviceReady(&hi2c1, (addr<<1), 1, 2))
			printf("%02X ",addr);
		else
			printf("-- ");
    } // for-loop
    printf("\n");
    return 0;
} // cl_i2c_scanner


