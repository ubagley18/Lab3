/*! @file PIT.c
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT).
 *
 *  This contains the functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-05-4
 */

#include "fsl_common.h"

#include "Types\types.h"
#include "PIT\PIT.h"


static uint32_t GlobalPITModuleClk; 	// module clock.
static void (*UserFunction)(void *); 	// user call back function.
static void * UserArguments; 			// user call back function arguments.



/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
	// values of param global
	GlobalPITModuleClk = moduleClk;
	UserFunction = userFunction;
	UserArguments = userArguments;

	// page --> 1117
	CLOCK_EnableClock(kCLOCK_Pit0); //Clock Enable
	PIT->MCR &= ~PIT_MCR_MDIS_MASK; //writing 0 to  MCR[MDIS] to activate the PIT module
	PIT->MCR &= PIT_MCR_FRZ_MASK; // if device enters debug mode, freeze the PIT timer

	PIT_Enable(true);

	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; // enabling timer interrupts.

	NVIC_ClearPendingIRQ(PIT0_IRQn); //clearing pending interrupts
	NVIC_EnableIRQ(PIT0_IRQn); // enable interrupts

	return true;
}

/*! @brief Sets the value of the desired period of the PIT.
 *
 *  @param period The desired value of the timer period in nanoseconds.
 *  @param restart TRUE if the PIT is disabled, a new value set, and then enabled.
 *                 FALSE if the PIT will use the new value after a trigger event.
 *  @note The function will enable the timer and interrupts for the PIT.
 */
void PIT_Set(const uint32_t period, const bool restart)
{
	//LDVAL register trigger Calculation --> formula from pg.1117

	uint32_t triggerValue = ((GlobalPITModuleClk * period)/1e9) - 1;

	PIT->CHANNEL[0].LDVAL = triggerValue;

	if(restart)
	{
		PIT_Enable(false);
		PIT_Enable(true);
	}

}

/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
void PIT_Enable(const bool enable)
{
	if(enable)
		PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; // timer enabled on channel 0 upon request
	else
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; // timer disabled on channel 0 upon request

}

