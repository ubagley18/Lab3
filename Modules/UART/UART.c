/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Uldis Bagley and Prashant Shreshtha
 *  @date 2020-03-18
 */
#include "UART.h"
#include "fsl_common.h"
#include "FIFO\FIFO.h"
#include "fsl_port.h"


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


	// Ensure both receiver and transmitter are disabled for startup
	UART0->C2 &= ~UART_C2_RE_MASK;
	UART0->C2 &= ~UART_C2_TE_MASK;

	// C1, C2, C3, C4, C5, BDH, BDL, MODEM, D, S1, S2 registers in UART0 need to be set
	// C1 has no features to be initiated
	UART0->C2 |= UART_C2_RE_MASK; // Activates the Receiver
	UART0->C2 |= UART_C2_TE_MASK; // Activates the Transmitter


	/*
	// Initiation comments
	// S1 has no features to be initiated
	// S2 has no features to be initiated
	// C3 has no features to be initiated
	// D has no features to be initiated
	// Use UART_C4_BRFA(x) where x is the value of the baud rate fine adjust
	 */

	// SBR and fine adjust calculations
	sbr.l = moduleClk / (16 * baudRate); // Fills union address with sbr value (whole number)

	brfd = ((moduleClk / ((float)16 * baudRate)) - (16 * sbr.l));
	brfa = brfd * 32;

	// Set SBR registers
	UART0->BDH |= UART_BDH_SBR(sbr.s.Hi);
	UART0->BDL |= UART_BDL_SBR(sbr.s.Lo);

	//Set BRFD
	UART0->C4 |= UART_C4_BRFA(brfa);

	//Initialise TxFIFO and RxFIFO
	FIFO_Init(&TxFIFO);
	FIFO_Init(&RxFIFO);

	return true;
}

bool UART_InChar(uint8_t* const dataPtr)
{
	if(FIFO_Get(&RxFIFO, dataPtr))
		return true;
	else
		return false;
}

bool UART_OutChar(const uint8_t data)
{
	if(FIFO_Put(&TxFIFO, data))
		return true;
	else
		return false;
}

void UART_Poll(void)
{
	// Poll the hardware receive flag, RDRF
	if (UART0->S1 & UART_S1_RDRF_MASK)
		FIFO_Put(&RxFIFO, UART0->D);

	// Poll the hardware transmit flag, TDRE
	if (UART0->S1 & UART_S1_TDRE_MASK)
		FIFO_Get(&TxFIFO, (uint8_t *)&UART0->D); // Gets data from TxFIFO if hardware is ready to transmit a packet
}
