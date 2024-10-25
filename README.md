# NUCLEO-F103RB_TM1637Test

## Libraries
    
    Adapted from Arduino Library:
    TM1637 by Avishay Orpaz, version 1.2.0, simple, clean, works
		
## Board / Connections
    
    Using STMicro NUCLEO-F103RB development board
    Connect TM1637 7-segment display pins to:
    PC10 (CLK)
    PC12 (DIO)
		
## Notes
    
    Although the CLK and DIO signals appear and function similar to I2C,
    this protocol IS NOT I2C.  I2C requires the device address to be sent first.
    This TM1637 does not have an I2C address, and assumes all communication
    on the CLK/DIO signals are for it.
