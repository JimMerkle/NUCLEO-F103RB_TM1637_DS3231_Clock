// TM1637_Interface.hpp
// C++ layer to initialize and test the TM1637 C++ class, TM1637Display.
#ifndef __TM1637INTERFACE__
#define __TM1637INTERFACE__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief  Basic GPIO class defining an STM32 GPIO pin (peripheral port / pin)
  * @param  GPIO_Port: where x can be (A..G depending on device used) to select the GPIO peripheral
  * @param  GPIO_Pin: specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  */
class STM32Gpio {
  public:
	// Constructor
	STM32Gpio(){m_GPIO_Port=NULL;m_GPIO_Pin=0;};
	STM32Gpio(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin){m_GPIO_Port=GPIO_Port;m_GPIO_Pin=GPIO_Pin;};
	// Member variables
	GPIO_TypeDef *m_GPIO_Port;
	uint16_t m_GPIO_Pin;
};

// The TM1637 library depends on the Arduino "pinMode()" and "digitalRead()" APIs
// To implement a translation layer, we will create a STM32Gpio object that can be passed like an Arduio pin.
extern void pinMode(STM32Gpio pin,uint8_t mode);

// Define Arduino digitalRead()
extern int digitalRead(STM32Gpio pin);

extern uint16_t timer_delay_us(uint16_t delay_us);

// Arduino pinMode values:
#define INPUT   0
#define OUTPUT  1

// Redefine Arduino API to use our own microsecond delay
#define delayMicroseconds timer_delay_us

void tm1637_test(void);
void init_tm1637(void);
void update_clock(void);

#ifdef __cplusplus
}
#endif

#endif // __TM1637INTERFACE__

