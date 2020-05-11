/*!
**  @addtogroup UART_module UART module documentation
**  @{
*/
/* MODULE UART */
/*! @file UART.c
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-03-18
 */
#include "UART.h"
#include "fsl_common.h"
#include "FIFO\FIFO.h"
#include "fsl_port.h"

//Transmitter is driven by baud rate clock divided by 16
//Receiver has acquisition rate of 16 samples per bit time
static const uint8_t SAMPLE_BAUD_RATE = 16;
//Baud Rate Fractional Divisor for baud rate of 38400 with 0.047% error
static const uint8_t BAUD_RATE_DIVISOR = 32;

const port_pin_config_t UART_PORT_PIN_CONFIG =
{
		.pullSelect = kPORT_PullDisable,
		.slewRate = kPORT_SlowSlewRate,
		.passiveFilterEnable = kPORT_PassiveFilterDisable,
		.openDrainEnable = kPORT_OpenDrainDisable,
		.driveStrength = kPORT_LowDriveStrength,
		// for PIN multiplexing see p. 248 of K64 document
		.mux = kPORT_MuxAlt3,
		.lockRegister = kPORT_UnlockRegister
};

//Globally declared transmit and receive FIFO
static TFIFO TxFIFO, //Put from packet and Get into UART output by setting TDRE
             RxFIFO; //When RDRF is set Put and Get from RxFIFO


bool UART_Init(const uint32_t moduleClk, const uint32_t baudRate)
{
	int16union_t sbr; // From types.h
	float brfd;
	uint8_t brfa;

	CLOCK_EnableClock(kCLOCK_Uart0);
	CLOCK_EnableClock(kCLOCK_PortB); // Enable clock to portB so we can configure it

	PORT_SetPinConfig(PORTB, 16, &UART_PORT_PIN_CONFIG);
	PORT_SetPinConfig(PORTB, 17, &UART_PORT_PIN_CONFIG);

	UART0->C2 |= UART_C2_RE_MASK; // Activates the Receiver
	UART0->C2 |= UART_C2_TE_MASK; // Activates the Transmitter

	UART0->C2 |= UART_C2_RIE_MASK;

	// SBR and fine adjust calculations
	sbr.l = moduleClk / (SAMPLE_BAUD_RATE * baudRate); // Fills union address with sbr value (whole number)

	brfd = ((moduleClk / ((float)SAMPLE_BAUD_RATE * baudRate)) - (SAMPLE_BAUD_RATE * sbr.l));
	brfa = brfd * BAUD_RATE_DIVISOR;

	// Set SBR registers
	UART0->BDH |= UART_BDH_SBR(sbr.s.Hi);
	UART0->BDL |= UART_BDL_SBR(sbr.s.Lo);

	//Set BRFD
	UART0->C4 |= UART_C4_BRFA(brfa);

	//Initialise TxFIFO and RxFIFO
	FIFO_Init(&TxFIFO);
	FIFO_Init(&RxFIFO);

	NVIC_ClearPendingIRQ(UART0_RX_TX_IRQn);  // Clear pending interrupts on the UART
	NVIC_EnableIRQ(UART0_RX_TX_IRQn); // Enable interrupts

	return true;
}

bool UART_InChar(uint8_t* const dataPtr)
{
	return FIFO_Get(&RxFIFO, dataPtr);
}

bool UART_OutChar(const uint8_t data)
{
	if (FIFO_Put(&TxFIFO, data))
	{
		//Enable TIE
		UART0->C2 &= UART_C2_TIE_MASK;
		// Check if data is ready for output
		if (UART0->S1 & UART_S1_TDRE_MASK)
		{
			if (FIFO_Get(&TxFIFO, (uint8_t *)&UART0->D)) // Gets data from TxFIFO if hardware is ready to transmit a packet
				return true;
			else
				return false;
		}
		else // Data must be outputting so we return true
			return true;
	}
	else
		return false;
}

void __attribute__ ((interrupt)) UART_ISR(void)
{
	// Receive a character
	if (UART0->C2 & UART_C2_RIE_MASK)
	{
		// Clear RDRF flag by reading the status register
		if (UART0->S1 & UART_S1_RDRF_MASK)
			FIFO_Put(&RxFIFO, UART0->D);
	}

	// Transmit a character
	if (UART0->C2 & UART_C2_TIE_MASK)
	{
		// Clear TDRE flag by reading the status register
		if (UART0->S1 & UART_S1_TDRE_MASK)
			if (!FIFO_Get(&TxFIFO, (uint8_t *)&UART0->D)); // Gets data from TxFIFO if hardware is ready to transmit a packet
				UART0->C2 ^= ~UART_C2_TIE_MASK; // if FIFO_Get returns false disable TIE
	}

}

/* END UART */
/*!
** @}
*/
