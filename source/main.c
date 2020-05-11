/*!
** @file main.c
** @version 3.0
** @brief
**         Main module.
**         This module implements serial port (USB) communications with a PC.
**         The Simple Serial Communication Protocol is used.
**         The command handlers for a several commands (defined in the protocol) are implemented.
**
** @author Uldis Bagley and Prashant Shrestha
** @date 2020-05-12
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */
#include "clock_config.h"
#include "pin_mux.h"


#include "Types\types.h"
#include "UART\UART.h"
#include "Packet\packet.h"
#include "Flash\Flash.h"
#include "LEDs\LEDs.h"
#include "FTM\FTM.h"
#include "RTC\RTC.h"
#include "PIT\PIT.h"



// Commands
#define PACKET_CMD_ACK 0x80
#define STARTUP_CMD 0x04
#define VERSION_CMD 0X09
#define NUMBER_CMD 0x0B
#define MODE_CMD 0x0D
#define FLASH_PROGRAM_CMD 0x07
#define FLASH_READ_CMD 0x08
#define TIME_CMD 0x0C

// Version number
const uint8_t VERSION_MAJOR = 0x01; //1
const uint8_t VERSION_MINOR = 0x00; //0


// Baud rate
const uint32_t BAUD_RATE = 115200;


// Public Global variables
TPacket Packet;


// Private global variables
static uint16union_t Mcu_Nb; // MCU number
static uint16union_t Mcu_Md; // MCU Mode

volatile uint16union_t *NvMCUNb;	// The non-volatile MCU number
volatile uint16union_t *NvMCUMd;	// The non-volatile MCU mode


// Function Prototypes

/*! @brief Sends startup packets to the PC.
 *
 *  @return bool - TRUE if sending the startup packets was successful.
 *  @note Assumes that MCUInit has been called successfully.
 */
static bool SendStartupPackets(void);


/*! @brief sets initial values for MCU number and MCU Mode.
 *
 *  @return bool - TRUE if allocation is successful.
 */
static bool FlashAllocation_Init(void);


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


/*! @brief Respond to a MCU Mode packet sent from the PC.
 *
 *  @return bool - TRUE if the packet was handled successfully.
 */
static bool HandleModePacket(void);


/*! @brief Programs a byte into Flash
 *
 *  @return bool - TRUE if byte written successfully
 */
static bool HandleFlashProgram(void);


/*! @brief Reads data from Flash
 *
 *  @return bool - TRUE if data read successfully
 */
static bool HandleFlashRead(void);


/*! @brief Respond to packets sent from the PC.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
static void HandlePackets(void);


/* @brief Toggles green LED.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
void PITCallback(void* arg);


/* @brief Turns off blue LED.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
void UARTTimerCallback(void* arg);


/* @brief Toggles red LED.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
void RTCCallback(void* arg);


/* @brief Toggles blue LED.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
void FTMCallback(void* arg);



static bool SendStartupPackets(void)
{
	Packet_Put(STARTUP_CMD, 0, 0, 0);
	Packet_Put(VERSION_CMD, 'v', VERSION_MAJOR, VERSION_MINOR);
	Packet_Put(NUMBER_CMD, 1, NvMCUNb->s.Lo, NvMCUNb->s.Hi);
	Packet_Put(MODE_CMD, 1, NvMCUMd->s.Lo, NvMCUMd->s.Hi);

	return true;
}

static bool FlashAllocation_Init()
{
	bool success;

	  if (Flash_AllocateVar((volatile void **)&NvMCUNb, sizeof(*NvMCUNb)))
	  {
	    if (NvMCUNb->l == 0xFFFF)
	    {
	      success = Flash_Write16((uint16_t volatile *) &NvMCUNb->l, Mcu_Nb.l);
	    }
	  }
	  else
	  {
	    success = false;
	  }
	  if (Flash_AllocateVar((volatile void **)&NvMCUMd, sizeof(*NvMCUMd)))
	  {
	    if (NvMCUMd->l == 0xFFFF)
	    {
	      success = Flash_Write16((uint16_t volatile *) &NvMCUMd->l, Mcu_Md.l);
	    }
	  }
	  else
	  {
	    success = false;
	  }
	  return success;
}


static bool MCUInit(void)
{
	bool init;

	BOARD_InitPins();
	BOARD_InitBootClocks();

	init =	Packet_Init(SystemCoreClock, BAUD_RATE) &&
			Flash_Init() &&
			LEDs_Init() &&
			FlashAllocation_Init() &&
			PIT_Init(CLOCK_GetFreq(kCLOCK_BusClk), PITCallback,NULL) &&
			RTC_Init(RTCCallback, NULL) &&
			FTM_Init();

	// SystemCoreClock from system_MK64F12.c
	if (init)
	{
		Mcu_Nb.l = 1291; // Init student number to fill union
		Mcu_Md.l = 1; // Init MCUMode

		return true;
	}

	return false;
}



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


static bool HandleVersionPacket(void)
{
	if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x') && (Packet_Parameter3 == 13))
		return Packet_Put(VERSION_CMD, 'v', VERSION_MAJOR, VERSION_MINOR);
	else
		return false;
}


static bool HandleNumberPacket(void)
{
	if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
		return Packet_Put(NUMBER_CMD, 1, NvMCUNb->s.Lo, NvMCUNb->s.Hi);

	else if ((Packet_Parameter1 == 2))
	{
    	//doing it the way Peter suggests in the lab2 manual
		Flash_Write16((uint16_t *)&NvMCUNb->l, Packet_Parameter23);
		Packet_Put(NUMBER_CMD, 2, Packet_Parameter2, Packet_Parameter3);

		return true;
	}
	else
		return false;
}

static bool HandleModePacket(void)
{
	if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
		return  Packet_Put(MODE_CMD, 1, NvMCUMd->s.Lo, NvMCUMd->s.Hi);

	else if ((Packet_Parameter1 == 2))
	{
		Flash_Write16((uint16_t *)&NvMCUMd->l, Packet_Parameter23);
		Packet_Put(MODE_CMD, 2, Packet_Parameter2, Packet_Parameter3);

		return true;
	}

	else
		return false;
}


static bool HandleFlashProgram(void)
{
	if ((Packet_Parameter1 >= 0) && (Packet_Parameter1 <= 7) && (Packet_Parameter2 == 0))
	{
		return Flash_Write8((uint8_t*)(FLASH_DATA_START + Packet_Parameter1), Packet_Parameter3);
	}

	else if ((Packet_Parameter1 == 8) && (Packet_Parameter2 == 0))
	{
		return Flash_Erase();
	}
	else
		return false;
}

static bool HandleFlashRead(void)
{

	if (Packet_Parameter1 >= 0 && Packet_Parameter1 <= 7 && Packet_Parameter2 == 0)
	{
		return Packet_Put(FLASH_READ_CMD,Packet_Parameter1,0,_FB(FLASH_DATA_START + Packet_Parameter1));
	}

	return false;
}

static bool HandleTimePackets()
{
	if ((Packet_Parameter1 >= 0 && Packet_Parameter1 <=23) && //Hours
		(Packet_Parameter2 >= 0 && Packet_Parameter2 <=59) && //Minutes
		(Packet_Parameter3 >= 0 && Packet_Parameter3 <=59)) //Seconds
	{
		RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
		return true;
	}

	return false;

}



/* @brief Respond to packets sent from the PC.
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

		case MODE_CMD:
			success = HandleModePacket();
			break;

		case FLASH_PROGRAM_CMD:
			success = HandleFlashProgram();
			break;

		case FLASH_READ_CMD:
			success = HandleFlashRead();
			break;
		case TIME_CMD:
			success = HandleTimePackets();

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

/* @brief Toggles green LED.
 *
 *  @note Assumes that MCUInit has been called successfully.
 */
void PITCallback(void* arg)
{
	LEDs_Toggle(LED_GREEN);
}

void UARTTimerCallback(void* arg)
{
	LEDs_Off(LED_BLUE);
}

void RTCCallback(void* arg)
{
	LEDs_Toggle(LED_RED);
}

void FTMCallback(void* arg)
{
	LEDs_Toggle(LED_BLUE);
}

/*!
 * @brief Main function
 */
int main(void)
{
	MCUInit();

	for (;;)
	{
		if (Packet_Get())
		{
			LEDs_On(LED_BLUE);
			//FTM_StartTimer(); // what args do I pass here? set up timer channel?
			HandlePackets();
		}
	}
}

/* END main */
/*!
** @}
*/
