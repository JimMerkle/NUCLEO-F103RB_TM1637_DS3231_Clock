# NUCLEO-F103RB_TM1637_DS3231_Clock

		
## Development Board - Notes
    
    Using STMicro NUCLEO-F103RB
    
## TM1637 - 7 segment display
    
    Connect TM1637 7-segment display pins to:
      PC10 (CLK)
      PC12 (DIO)
    
    Although the TM1637's CLK and DIO signals appear similar to I2C,
    this protocol IS NOT I2C.  I2C requires the device address to be sent first.
    The TM1637 does not have an I2C address, and assumes all communication
    on the CLK/DIO signals are for it.
    
## TM1637 Library
      
    Adapted from Arduino Library:
    TM1637 by Avishay Orpaz, version 1.2.0, simple, clean, works
       
## DS3231 - Connections
    
    Connect DS3231 to I2C1 pins:
     PB8 (SCL)
     PB9 (SDA)
		
## 1us delay timer
    
    Using 16-bit timer, TIM4, to count 1us time increments
    This is to provide pulse width support for the TM1637 library,
    originally provided by Arduino API - void delayMicroseconds(unsigned int us);
    Prescaler is adjusted such that timer increments once each microsecond.
    
## Using STM32's RTC for real-time management
    
    The RTC module implemented within the STM32-F103RB is a 32-bit counter
    that increments each second.  The STM32 HAL provides an interface for
    years, months, days, hours, minutes, seconds (normal RTC stuff),
    allowing the developer to work with these normal units.
    Be sure to remove/rotate 45 degrees the charging diode.
    Don't want your CR2032 being destroyed!
    
## Using DS3231 RTC for real-time management
    
    JeeLab wrote an excellent RTC library for Arduino,
    supporting multiple RTC modules as well as functions to convert from
    Unix time (32-bit number of seconds) to RTC (hours, minutes, seconds).
    https://github.com/adafruit/RTClib/blob/master/src/RTClib.cpp
    Much of the "utility code" has been copied to RTClib.c.
    
## Notes
    

