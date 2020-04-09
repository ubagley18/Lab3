/*! @file LEDs.c
 *
 *  @brief Routines to access the LEDs on the FRDM-K64F.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-02-11
 */

// new types
#include "LEDs.h"

/*! @brief Sets up the LEDs before first use.
 *
 *  @return bool - TRUE if the LEDs were successfully initialized.
 */
bool LEDs_Init(void)
{

}

/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const LED_t color)
{

}

/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const LED_t color)
{

}

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const LED_t color)
{

}
