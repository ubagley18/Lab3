
#include "fsl_common.h"

// new types
#include "Types\types.h"
#include "FTM.h"


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
		uint32_t CnSc_msa;
		CnSc_msa = FTM_CnSC_MSA(aFTMChannel->channelNb)& FTM_CnSC_MSA_MASK;
		FTM_CnSC_MSB(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;

	}
	else
	{

	}
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

}
