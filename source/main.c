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
#define MAJOR_VERSION_NB 1
#define MINOR_VERSION_NB 0

// Baud rate
// TODO: Define the UART baud rate using const.
//#define const baudRate 38400

// Commands
// TODO: Define the commands using enum.
enum Packet_Command
{
	STARTUP_CMD = 0x04,
	VERSION_CMD = 0X09,
	NUMBER_CMD = 0x0B
};

enum Packet_Command_ACK
{
	STARTUP_CMD = 0x84,
	VERSION_CMD = 0X89,
	NUMBER_CMD = 0x8B
};

// ----------------------------------------
// Private global variables
// TODO: Define the MCU number.
uint16union_t MCU_Nb = 1291;

/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void)
{
  // TODO: Send startup packets to the PC.
	// See Simple Serial Communication Protocol for parameters
	if (Packet_Put(STARTUP_CMD, 0, 0, 0) &&
			Packet_Put(VERSION_CMD, 'v', MAJOR_VERSION_NB, MINOR_VERSION_NB) &&
			Packet_Put(NUMBER_CMD, 1, MCU_Nb.s.Lo, MCU_Nb.s.Hi))
		return true;
	else
		return false;
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

}

/*! @brief Respond to a Startup packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool HandleStartupPacket(void)
{
  // TODO: Respond to a startup packet sent from the PC
	if (Packet_Parameter1 && Packet_Parameter2 && Packet_Parameter3 == 0)
	{
		SendStartupPackets();
		return true;
	}
	else
		return false;

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
  // 2. "switch" on the Pcaket_Command, but ignore the top bit.
  // 3. Create a case for each command, which calls the individual command handler.
  // e.g.
  //      case CMD_STARTUP:
  //      success = HandleStartupPacket();
  //      break;
  // 4. Check whether an ACK was requested from the PC.
  // 5. If an ACK was requestedm then send an ACK/NAK packet based on the success of the command handler.

	bool success;

	Packet_Get();

	switch (Packet_Command)
	{
		case STARTUP_CMD:
			success = HandleStartupPacket();
		break;
		case VERSION_CMD:
			success = HandleVersionPacket();
		break;
		case NUMBER_CMD:
			success = HandleNumberPacket();
		break;
	}

	if (success) //If success returns true then the packet has been handled successfully - so we return with an ACK
	{
		Packet_Command |= Packet_Command_ACK
		return Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
	}
	else
		return Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);

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
    // 2. Handle any packets received.
  	if(Packet_Get())
  		HandlePackets();

  	UART_Poll();
  }
}

/* END main */
/*!
** @}
*/
