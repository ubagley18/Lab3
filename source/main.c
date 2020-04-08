/*!
** @file main.c
** @version 1.0
** @brief
**         Main module.
**         This module implements serial port (USB) communications with a PC.
**         The Simple Serial Communication Protocol is used.
**         The command handlers for a several commands (defined in the protocol) are implemented.
**
** @author Uldis Bagley and Prashant Shrestha
** @date 2020-04-7
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


// Commands
#define PACKET_CMD_ACK 0x80
#define STARTUP_CMD 0x04
#define VERSION_CMD 0X09
#define NUMBER_CMD 0x0B

// Version number
const uint8_t VERSION_MAJOR = 0x01; //1
const uint8_t VERSION_MINOR = 0x00; //0

// Baud rate
const uint16_t BAUD_RATE = 38400;


// Public Global variables
TPacket Packet;


//Private global variables
static uint16union_t Mcu_Nb;


//Function Prototypes

/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void);


/*! @brief Initializes the MCU by initializing all variables and then sending startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 */
static bool MCUInit(void);


/*! @brief Respond to a Startup packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool HandleStartupPacket(void);


/*! @brief Respond to a Version packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 */
static bool HandleVersionPacket(void);


/*! @brief Respond to a MCU Number packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 */
static bool HandleNumberPacket(void);


/*! @brief Respond to packets sent from the PC.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
static void HandlePackets(void);


/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void)
{
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

  Packet_Init(SystemCoreClock, BAUD_RATE);// SystemCoreClock from system_MK64F12.c
  Mcu_Nb.l = 1291; // Init student number to fill union

  return true;
}

/*! @brief Respond to a Startup packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool HandleStartupPacket(void)
{
  if ((Packet_Parameter1 == 0) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    SendStartupPackets();
	return true;
  }
  else
  	return false;
}

/*! @brief Respond to a Version packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 */
static bool HandleVersionPacket(void)
{
  if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x') && (Packet_Parameter3 == 13))
    return Packet_Put(VERSION_CMD, 'v', VERSION_MAJOR, VERSION_MINOR);
  else
  	return false;
}

/*! @brief Respond to a MCU Number packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 */
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

/*! @brief Respond to packets sent from the PC.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
static void HandlePackets(void)
{
  bool success;
  uint8_t command = Packet_Command; // Store the value of Packet_Command so ACK may be manipulated

  // If the command has an ACK request, remove it for the switch statement
  if ((Packet_Command & PACKET_CMD_ACK) == PACKET_CMD_ACK)
    Packet_Command &= ~PACKET_CMD_ACK;

  // Handle packets
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

  //If success returns true then the packet has been handled successfully - we return with an ACK if it was requested
  if ((success) && ((command & PACKET_CMD_ACK) == PACKET_CMD_ACK))
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
    UART_Poll();

    if (Packet_Get())
      HandlePackets();
  }
}

/* END main */
/*!
** @}
*/
