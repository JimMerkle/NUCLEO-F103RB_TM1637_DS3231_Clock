// DS3231.h, DS3231 library - minimal "C" functionality by Jim Merkle

#ifndef __DS3231_H__
#define __DS3231_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "RTClib.h" // DATE_TIME definition
#include "main.h" // HAL includes

#define DS3231_ADDRESS	0x68	// 7-bit address (does not include I2C R/W bit)

//=============================================================================
#define SMALL_HAL_TIMEOUT	40	// milliseconds
// Implement a "generic I2C API" for writing to and then reading from an I2C device (in that order)
HAL_StatusTypeDef i2c_write_read(uint16_t DevAddress, uint8_t * write_data, uint16_t write_count, uint8_t * read_data, uint16_t read_count);
HAL_StatusTypeDef init_ds3231(void);
HAL_StatusTypeDef read_ds3231(DATE_TIME * dt);
HAL_StatusTypeDef write_ds3231(DATE_TIME * dt);
//=============================================================================



#ifdef __cplusplus
}
#endif

#endif // __DS3231_H__
