/*! @file LEDs.c
 *
 *  @brief Routines to access the LEDs on the FRDM-K64F.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-04-27
 */
#include "fsl_gpio.h"
#include "fsl_port.h"

// new types
#include "LEDs.h"

const port_pin_config_t LED_PORT_PIN_CONFIG =
{
		.pullSelect = kPORT_PullDisable,
		.slewRate = kPORT_SlowSlewRate,
		.passiveFilterEnable = kPORT_PassiveFilterDisable,
		.openDrainEnable = kPORT_OpenDrainDisable,
		.driveStrength = kPORT_LowDriveStrength,
		.mux = kPORT_MuxAsGpio, //for pin multiplexing see page 248
		.lockRegister = kPORT_UnlockRegister
};

const gpio_pin_config_t LED_GPIO_CONFIG =
{
		kGPIO_DigitalOutput,
		0,
};


/*! @brief Sets up the LEDs before first use.
 *
 *  @return bool - TRUE if the LEDs were successfully initialized.
 */
bool LEDs_Init(void)
{
	CLOCK_EnableClock(kCLOCK_PortE);
	CLOCK_EnableClock(kCLOCK_PortB);

	PORT_SetPinConfig(PORTE,26, &LED_PORT_PIN_CONFIG);
	PORT_SetPinConfig(PORTB,21, &LED_PORT_PIN_CONFIG);
	PORT_SetPinConfig(PORTB,22, &LED_PORT_PIN_CONFIG);

	GPIO_PinInit(GPIOE, 26, &LED_GPIO_CONFIG);
	GPIO_PinInit(GPIOB, 21, &LED_GPIO_CONFIG);
	GPIO_PinInit(GPIOB, 22, &LED_GPIO_CONFIG);

	return true;
}

//use array to const array inside that, store, port config  for each LED.
//

/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const LED_t color)
{
	GPIO_PortClear(GPIOE,0x4000000); // setting port 26 to 0
}

/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const LED_t color)
{
	GPIO_PortSet(GPIOE,0x4000000); // setting port 26 to 1

}

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const LED_t color)
{
	GPIO_PortToggle(GPIOE,0x4000000);
}
