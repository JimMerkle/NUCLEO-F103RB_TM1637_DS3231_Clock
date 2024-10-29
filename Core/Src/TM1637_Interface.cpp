// TM1637_Interface.cpp
// C++ layer to initialize and test the TM1637 C++ class, TM1637Display.
#include "main.h" // GPIO pin definitions
#include <stdio.h> // printf()
#include <TM1637_Interface.h>
#include <TM1637Display.h>

// The TM1637 library depends on the Arduino "pinMode()" and "digitalRead()" APIs
// To implement a translation layer, we will create a STM32Gpio object that can be passed like an Arduio pin.
extern void pinMode(STM32Gpio pin,uint8_t mode);

// Define Arduino digitalRead()
extern int digitalRead(STM32Gpio pin);

/* Define GPIO pins : TM1637_CLK_Pin TM1637_DIO_Pin for STM32Gpio class objects */
STM32Gpio TM1637_CLK(TM1637_DIO_GPIO_Port, TM1637_CLK_Pin);
STM32Gpio TM1637_DIO(TM1637_DIO_GPIO_Port, TM1637_DIO_Pin);

// Using a 16 bit timer spin-delay a quantity of micro-seconds
// Timer is configured to increment each micro-second
// This function appears to work perfectly at 64-72MHz system clock, always returning 1000us, when 1000us was requested
//  - Release build only.  Debug build runs noticeably slower, returning values greater than what was expected.
// With 16MHz system clock and 8MHz peripheral clock, the delta times are 1000, 1001, and 1019 when systick interrupts fire
// With 8MHz system clock and 8MHz peripheral clock, the delta times are 1000, 1002, and 1033, 1036, 1038 when systick interrupts fire
uint16_t timer_delay_us(uint16_t delay_us)
{
    //printf("%s(%lu)\n",__func__,delay_us);
    volatile TIM_TypeDef *TIMx = TIM4; // Establish pointer to timer 4 registers
    uint16_t start_us = TIMx->CNT; // function entry count
    uint16_t delta;
    do {
    	delta = TIMx->CNT - start_us;
    } while(delta < delay_us);

    return delta;
}

void tm1637_test(void)
{
	// Initialize - construct the TM1637Display class
	TM1637Display display(TM1637_CLK, TM1637_DIO,100);

	uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
	display.setBrightness(0x0f);
	display.clear();
	HAL_Delay(500);
	// All segments on
	display.setSegments(data);
	HAL_Delay(500);
	display.showNumberDec(1234, true); // Expect: 1234
	HAL_Delay(500);
	display.showNumberDec(5678, true); // Expect: 5678

//	// Test TIM4
//	// In theory, TIM4 should be counting micro-second increments
//	// Have it time HAL_Delay(10).  The result should be between 10,000, and 11,000
//	// The following code produces: "HAL_Delay(10) took 10753 us"
//	uint16_t start_us = TIM4->CNT;
//	HAL_Delay(10);
//	uint16_t stop_us = TIM4->CNT;
//	printf("HAL_Delay(10) took %u us\r\n",stop_us - start_us);

//	// Count (and display count) as fast as possible
//	for(unsigned count=1;;count++)
//	{
//		display.showNumberDec(count, false);
//	}

//	// Loop, counting "seconds" on the display
//	// This makes a poor timer...  Just something to look at...
//	for(unsigned count=1;;count++)
//	{
//		display.showNumberDec(count, false);
//		HAL_Delay(1000);
//	}

	// Loop, counting "seconds" on the display
	// Use SysTick counter for milliseconds
	// Although this code counts seconds as accurately as the HSE clock provides,
	// it will roll over at 4,294,967,296 ticks (4,294,967 seconds) (49.7 days).
	// The counter roll-over isn't accounted for...  Use deltas can correct this...
//	uint32_t previous_ticks = HAL_GetTick();
//	for(unsigned count=1;;count++)
//	{
//		display.showNumberDec(count, false);
//		uint32_t leave_loop_ticks = previous_ticks + 1000;
//		while(HAL_GetTick() < leave_loop_ticks); // spin for 1 second
//		previous_ticks = leave_loop_ticks;
//	}

	// Similar code to the above, implementing deltas
	// Check the math....  Assume HAL_GetTick() returns 4,294,967,000 (0xFFFF FED8) and will roll over during the next 1000ms...

	uint32_t previous_ticks = HAL_GetTick();
	for(unsigned count=1;;count++)
	{
		display.showNumberDec(count, false);
		uint32_t leave_loop_ticks = previous_ticks + 1000; // if previous_ticks was 4,294,967,000, leave_loop_ticks will be 0x2C0 (704)
		while(leave_loop_ticks - HAL_GetTick() > 0); // spin for 1 second (0x2C0 - 0xFFFFFED8 = 0x3E8) The delta math does the overflow correctly
		previous_ticks = leave_loop_ticks;
	}

}


