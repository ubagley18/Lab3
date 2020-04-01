/*!
** @file
** @version 1.0
** @brief
**         Main module.
**         This module implements serial port (USB) communications with a PC.
**         The Simple Serial Communication Protocol is used.
**         The command handlers for a several commands (defined in the protocol) are implemented.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */

#include "clock_config.h"
#include "pin_mux.h"

// New types
#include "Types\types.h"

// Serial Communication Interface
#include "UART\UART.h"

// Packet handling
#include "Packet\packet.h"

// Version number
// TODO: Define the major version number and minor version number using const.

// Baud rate
// TODO: Define the UART baud rate using const.

// Commands
// TODO: Define the commands using enum.

// ----------------------------------------
// Private global variables
// TODO: Define the MCU number.

/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void)
{
	// indicate  success --> bool success
  // TODO: Send startup packets to the PC.
	//success = Packet_Put(CMD_STARTUP,0,0,0);
	//create similar packets as above for other packets for version of the software and MCU number
}

/*! @brief Initializes the MCU by initializing all variables and then sending startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 */
static bool MCUInit(void)
{
  BOARD_InitPins();
  BOARD_InitBootClocks();

  // TODO: Initialize any modules that need to be initialized.
  //Packet_Init(...); this will then call UART init...which will then call FIFO init
}

/*! @brief Respond to a Startup packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool HandleStartupPacket(void)
{
  // TODO: Respond to a startup packet sent from the PC
	// call Packet_Put(CMD_STARTUP); // not a good idea to call this here
	// probably a better way to do it --> SendStartupPackets();
}

// TODO: Create individual functions to handle each command sent from the PC, similar to HandleStartupPacket above.

/*! @brief Respond to packets sent from the PC.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
static void HandlePackets(void)
{
  // TODO: Create a packet handler


  // Pseudocode:
  // 1. Get a packet.


	//packet get returns boolean to say whether it was successful or not
	/*
	 * if(Packet_Get())
	 * {
	 * switch(Packet_Comand & 0x7F)    handle the packet --> use individual command handlers
	 * 	{
	 * 	case CMD_STARTUP:
       	   success = HandleStartupPacket();
       	   break;
	 * 	}
	 * 	// Handle Packet ack based on the result i.e.'success'
	 * 	if ack was asked for
	 * 	if(success)
	 * 	{
	 * 		send back an ack
	 * 	}
	 * 	else{
	 * 		nak
	 * 	}
	 * }
	 * */


  // 2. "switch" on the Pcaket_Command, but ignore the top bit.
  // 3. Create a case for each command, which calls the individual command handler.
  // e.g.
  //      case CMD_STARTUP:
  //      success = HandleStartupPacket();
  //      break;
  // 4. Check whether an ACK was requested from the PC.
  // 5. If an ACK was requestedm then send an ACK/NAK packet based on the success of the command handler.

}

/*!
 * @brief Main function
 */
int main(void)
{
  MCUInit();
  for (;;)
  {
    // TODO:
    // 1. Poll the UART.
	  //UART_Poll(); // brings in characters into the FIFO (RxFIFO)
    // 2. Handle any packets received.
	  //might need to create a private function --> HandlePackets();
  }
}

/* END main */
/*!
** @}
*/
