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

#define PACKET_CMD_ACK 0x80

//Function Prototypes
static bool SendStartupPackets(void);
static bool HandleStartupPacket(void);
static void HandlePackets(void);
static bool HandleVersionPacket(void);
static bool HandleNumberPacket(void);

// Version number
// TODO: Define the major version number and minor version number using const.
const uint8_t VERSION_MAJOR = 0x01; //1
const uint8_t VERSION_MINOR = 0x00; //0


// Baud rate
// TODO: Define the UART baud rate using const.
const uint16_t BAUD_RATE = 38400;

// Commands
// TODO: Define the commands using enum.
enum Packet_Command
{
	STARTUP_CMD = 0x04,
	VERSION_CMD = 0X09,
	NUMBER_CMD = 0x0B
};

// ----------------------------------------
// Private global variables
// TODO: Define the MCU number.
static uint16union_t Mcu_Nb;

/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void)
{
  // TODO: Send startup packets to the PC.
	// See Simple Serial Communication Protocol for parameters
	Packet_Put(STARTUP_CMD, 0, 0, 0);
	Packet_Put(VERSION_CMD, 'v', VERSION_MAJOR, VERSION_MINOR);
	Packet_Put(NUMBER_CMD, 1, Mcu_Nb.s.Lo, Mcu_Nb.s.Hi);
	return true;
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
  Packet_Init(SystemCoreClock, BAUD_RATE);// SystemCoreClock from system_MK64F12.c
  Mcu_Nb.l = 1291;

  return true;
}

/*! @brief Respond to a Startup packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool HandleStartupPacket(void)
{
  // TODO: Respond to a startup packet sent from the PC
	if (Packet_Parameter1 == 0)
	{
		SendStartupPackets();
		return true;
	}
	else
		return false;

}

static bool HandleVersionPacket(void)
{
  // TODO: Respond to a startup packet sent from the PC
	// call Packet_Put(CMD_STARTUP); // not a good idea to call this here
	// probably a better way to do it --> SendStartupPackets();
	if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x') && (Packet_Parameter3 == 13))
		return Packet_Put(VERSION_CMD, 'v', VERSION_MAJOR, VERSION_MINOR);
	else
		return false;
}

static bool HandleNumberPacket(void)
{
	uint16union_t new_Mcu_Nb;
	new_Mcu_Nb.s.Lo = Packet_Parameter2;
	new_Mcu_Nb.s.Hi = Packet_Parameter3;

	if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
		return Packet_Put(NUMBER_CMD, 1, Mcu_Nb.s.Lo, Mcu_Nb.s.Hi);
	else if ((Packet_Parameter1 == 2) && (Packet_Parameter2 == new_Mcu_Nb.s.Lo) && (Packet_Parameter3 == new_Mcu_Nb.s.Hi))
	{
		// Replace old Nb with new Nb
		Mcu_Nb.s.Lo = new_Mcu_Nb.s.Lo;
		Mcu_Nb.s.Hi = new_Mcu_Nb.s.Hi;
		return Packet_Put(NUMBER_CMD, 2, Mcu_Nb.s.Lo, Mcu_Nb.s.Hi);
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
	//1 0000 0100
	//1 0000 0000
	//0 0000 0001
	uint8_t command = Packet_Command; // Store the value of Packet_Command so ACK may be manipulated

	if ((Packet_Command & PACKET_CMD_ACK) == PACKET_CMD_ACK) // If the command has an ACK request, remove it for the switch statement
		Packet_Command &= ~PACKET_CMD_ACK;

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
			default:
				return;
		}

	if ((success) && ((command & PACKET_CMD_ACK) == PACKET_CMD_ACK)) //If success returns true then the packet has been handled successfully - we return with an ACK if it was requested
	{
		Packet_Put(command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
	}
	else
		return;


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
  	if (Packet_Get())
  		HandlePackets();

  	UART_Poll();
  }
}

/* END main */
/*!
** @}
*/
