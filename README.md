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
    
## Notes
    
    Although the CLK and DIO signals appear and function similar to I2C,
    this protocol IS NOT I2C.  I2C requires the device address to be sent first.
    This TM1637 does not have an I2C address, and assumes all communication
    on the CLK/DIO signals are for it.
