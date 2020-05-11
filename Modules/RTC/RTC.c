/*!
**  @addtogroup RTC_module RTC module documentation
**  @{
*/
/* MODULE RTC */
/*! @file RTC.c
 *
 *  @brief Routines for controlling the Real Time Clock (RTC).
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-05-09
 */
#include "RTC.h"
#include "fsl_common.h"
#include "fsl_port.h"


static void (*RTC_Callback)(void*);
static void *RTC_UserArguments;

//RTC_TSR when time counter is enabled TSR is read only and increments once per second and sets off interrupt

/*RTC_CR_OSCE when 1 , 32.768 kHz oscillator is enabled. After setting this bit, wait the oscillator startup time before enabling
the time counter to allow the 32.768 kHz clock time to stabilize*/

//RTC_CR_UM set 1 to allow writes to TCE when locked

//RTC_SR_TCE when 1 TSR and TPR are not writable but increment

//RTC_LR_CRL when 0 control register locks




bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  // Assign callback function and its arguments to private global variables
  RTC_Callback = userFunction;
  RTC_UserArguments = userArguments;

  // Enable clock gate
  CLOCK_EnableClock(kCLOCK_Rtc0);

  // Enable necessary capacitors for the required load capacitance (18pF)
  RTC->CR |= RTC_CR_SC2P_MASK;   //2pF
  RTC->CR |= RTC_CR_SC16P_MASK;  //16pF

  // Check Time Invalid Flag and clear if set
  if (RTC->SR & RTC_SR_TIF_MASK)
  {
	RTC_Set(0, 0, 0);
  }

  // Enable Time Seconds Interrupt
  RTC->IER |= RTC_IER_TSIE_MASK;

  // Enable Time Counter (this will make the TSR and TPR registers increment but non-writable)
  RTC->SR |= RTC_SR_TCE_MASK;

  // Enable Oscillator clock of 32.768 kHz
  RTC->CR |= RTC_CR_OSCE_MASK;

  // Then wait oscillator startup time before enabling TCE
  for (int i = 0; i <= 100000; i++) {} //1000ms in us

  // Enable Time Counter (this will make the TSR and TPR registers increment but non-writable)
  RTC->SR |= RTC_SR_TCE_MASK;

  NVIC_ClearPendingIRQ(RTC_IRQn);  // Clear pending interrupts on the RTC timer
  NVIC_EnableIRQ(RTC_IRQn);  // Enable RTC Interrupt Service Routine

  return true;
}


void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
	uint32_t timeInSeconds; // combined time of hours, minutes and seconds in seconds

	RTC->SR &= ~RTC_SR_TCE_MASK; // disable time counter so TSR can be written to

	// Convert time set inputs into seconds and load into TSR register
	timeInSeconds = (hours*3600) + (minutes*60) + seconds;
	RTC->TSR = timeInSeconds;

	RTC->SR |= RTC_SR_TCE_MASK; // restart counter with new time
}


void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
	uint32_t timeInSeconds = RTC->TSR;

	// Take modulo to ensure no effect from time > 24 hrs
	// Acquire the value of hours in seconds and convert into hours(0-23)
	*hours = (uint8_t) ((timeInSeconds) / 3600 % 24);
	// Acquire the value of minutes in seconds and convert into minutes(0-59)
	*minutes = (uint8_t) ((timeInSeconds) / 60 % 60);
	// Acquire the seconds
	*seconds = (uint8_t) (timeInSeconds % 60);
}

/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The user callback function will be called.
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) RTC_ISR(void){

  //call the function that was passed in RTC init
  if(RTC_Callback){
      RTC_Callback(RTC_UserArguments);
  }

};

/* END RTC */
/*!
** @}
*/
