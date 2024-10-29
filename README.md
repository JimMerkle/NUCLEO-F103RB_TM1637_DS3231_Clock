# NUCLEO-F103RB_TM1637Test

## Libraries
    
    Adapted from Arduino Library:
    TM1637 by Avishay Orpaz, version 1.2.0, simple, clean, works
		
## Board / Connections
    
    Using STMicro NUCLEO-F103RB development board
    Connect TM1637 7-segment display pins to:
    PC10 (CLK)
    PC12 (DIO)
		
## 1us delay timer
    
    Using 16-bit timer, TIM4, to count 1us time increments
    This is to provide pulse width support originally provided by Arduino API
    void delayMicroseconds(unsigned int us);
    Prescaler is adjusted such that timer increments once each microsecond.
    
## Using STM32's RTC for real-time management
    
    The RTC module implemented within the STM32-F103RB is a 32-bit counter
    that increments each second.  It is up to the user to decode
    years, months, days, hours, minutes, seconds (Normal RTC stuff).
    
    Fortunately for us, JeeLab wrote an excellent RTC library for Arduino,
    supporting multiple RTC modules as well as functions to convert from
    Unix time (32-bit number of seconds) to RTC (hours, minutes, seconds).
    https://github.com/adafruit/RTClib/blob/master/src/RTClib.cpp
    Much of the "utility code" has been copied here, to RTClib.c.
    
## Notes
    
    Although the CLK and DIO signals appear and function similar to I2C,
    this protocol IS NOT I2C.  I2C requires the device address to be sent first.
    This TM1637 does not have an I2C address, and assumes all communication
    on the CLK/DIO signals are for it.
