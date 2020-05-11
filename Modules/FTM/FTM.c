/*!
**  @addtogroup FTM_module FTM module documentation
**  @{
*/
/* MODULE FTM */
/*! @file FTM.c
 *
 *  @brief Routines for setting up the FlexTimer module (FTM).
 *
 *  This contains the functions for operating the FlexTimer module (FTM).
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-05-11
 */

// new types
#include "Types\types.h"
#include "FTM.h"

#include "fsl_common.h"


static void (*UserFunction)(void *);
static void * UserArguments;

/*! @brief Sets up the FTM before first use.
 *
 *  Enables the FTM as a free running 16-bit counter.
 *  @return bool - TRUE if the FTM was successfully initialized.
 */
bool FTM_Init()
{
	CLOCK_EnableClock(kCLOCK_Ftm0); //FTM module clock enabled

	FTM0->CNTIN = ~FTM_CNTIN_INIT_MASK; // FTM initial value set to 0x0000
	FTM0->MOD = FTM_MOD_MOD_MASK; //writing 0xFFFF to Modulo Register
	FTM0->CNT = ~FTM_CNT_COUNT_MASK; // setting counter to 0 by writing 0x0000 to counter register --> page 988 && 989
	FTM0->SC = FTM_SC_CLKS(0x02);// Fixed Frequency clock selected as clock source  --> page 988.

	NVIC_ClearPendingIRQ(FTM0_IRQn); //clearing pending interrupts
	NVIC_EnableIRQ(FTM0_IRQn); // enable interrupts


	return true;

}

/*! @brief Sets up a timer channel.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *    channelNb is the channel number of the FTM to use.
 *    delayCount is the delay count (in module clock periods) for an output compare event.
 *    timerFunction is used to set the timer up as either an input capture or an output compare.
 *    ioType is a union that depends on the setting of the channel as input capture or output compare:
 *      outputAction is the action to take on a successful output compare.
 *      inputDetection is the type of input capture detection.
 *    callbackFunction is a pointer to a user callback function.
 *    callbackArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the timer was set up successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
	if (aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)
	{
		FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_MSA_MASK;
		FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_MSB_MASK;
	}
	else
	{
		FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_MSB_MASK;
		FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_MSA_MASK;
	}

	//from page.990
	switch (aFTMChannel->ioType.inputDetection)
	{
		case 1: //capture on rising edge only
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_ELSB_MASK; // 0
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_ELSA_MASK;  // 1
			break;
		case 2: // capture on falling edge only
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_ELSB_MASK; 	// 1
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_ELSA_MASK;	// 0
			break;
		case 3: // capture on rising or falling edge
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_ELSB_MASK; 	// 1
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_ELSA_MASK; 	// 1
			break;
		default: // revert to GPIO as pin not used for FTM
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_ELSB_MASK; // 0
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_ELSA_MASK;	// 0
			break;

	}

	//global user function and arguments
	UserFunction = aFTMChannel->callbackFunction;
	UserArguments = aFTMChannel->callbackArguments;

	return true;
}


/*! @brief Starts a timer if set up for output compare.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *  @return bool - TRUE if the timer was started successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
	if (aFTMChannel->channelNb < 8 ) //valid channel?
	{
		if (aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
		{
			FTM0->CONTROLS[aFTMChannel->channelNb].CnSC |= FTM_CnSC_CHIE_MASK; // channel interrupt enable
			FTM0->CONTROLS[aFTMChannel->channelNb].CnV = FTM0->CNT + aFTMChannel->delayNanoseconds; // channel counter initialized.
			return true;
		}
	}

	return false;
}

void FTM0_IRQHandler(void)
{

	//clear flags? CONTROLS[?]
	//FTM0->CONTROLS[aFTMChannel->channelNb].CnSC &= ~FTM_CnSC_CHF_MASK;

	//call user function
	if(UserFunction)
	{
		(*UserFunction)(UserArguments);
	}
}

/* END FTM */
/*!
** @}
*/
