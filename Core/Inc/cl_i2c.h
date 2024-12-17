// Copyright Jim Merkle, 2/17/2020
// File: cl_i2c.h
//
// Defines, typedefs, structures for cl_i2c.c module
//
#ifndef _CL_I2C_H_
#define _CL_I2C_H_

#include "main.h"          // HAL functions and defines
#include "command_line.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_ADDRESS_MIN	0x03
#define I2C_ADDRESS_MAX 0x77
#define HAL_I2C_SMALL_TIMEOUT 50

// Prototypes:
int cl_i2c_validate_address(uint16_t i2c_address); // I2C helper function that validates I2C address is within range
HAL_StatusTypeDef i2c_write_read(uint16_t DevAddress, uint8_t * write_data, uint16_t write_count, uint8_t * read_data, uint16_t read_count);
int cl_i2c_scan(void);
int cl_i2c_write(void);
int cl_i2c_read(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _CL_I2C_H_ */
