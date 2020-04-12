/*! @file Flash.c
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-04-09
 */

#include "Flash.h"

#include "MK64F12.h"

//FTFE_Type TFTFE;

typedef struct
{
  uint8_t Command;   /* FCMD (defines FTFE command) */
  uint8_t Address1; /* FlashAddress [23:16] */
  uint8_t Address2; /* FlashAddress [15:8]  */
  uint8_t Address3; /* FlashAddress [7:0]   */
  uint8_t DataByte0;
  uint8_t DataByte1;
  uint8_t DataByte2;
  uint8_t DataByte3;
  uint8_t DataByte4;
  uint8_t DataByte5;
  uint8_t DataByte6;
  uint8_t DataByte7;
} FCCOB_t;

////Prototype
//static bool LaunchCommand(FCCOB_t* commonCommandObject);
//static bool WritePhrase(const uint32_t address, const uint64union_t phrase);
//static bool EraseSector(const uint32_t address);
//static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);

static bool LaunchCommand(FCCOB_t* commonCommandObject)
{
	//check section 29.4.10.1.3
	//Flash Access Error Flag 			--> 			ACCERR 	--> 	0 No, 1 Yes
	//Command Complete Interrupt Flag	 --> 			CCIF 	-->		0 in Progress, 1 Complete
	//Flash Protection Violation Flag	-->				FPVIOL	-->		0 No, 1 Protection detected
	while (!(FTFE_FSTAT_CCIF_MASK & FTFE->FSTAT))
	{

	}

	if ((FTFE_FSTAT_ACCERR_MASK & FTFE->FSTAT)
		|| (FTFE_FSTAT_FPVIOL_MASK & FTFE->FSTAT)) //To access error and protection violation check
	{
		FTFE->FSTAT = 0x30;        //set FSTAT = 0x30
	}

	FTFE->FCCOB0 = commonCommandObject->Command;
	FTFE->FCCOB1 = commonCommandObject->Address1;
	FTFE->FCCOB2 = commonCommandObject->Address2;
	FTFE->FCCOB3 = commonCommandObject->Address3;
	FTFE->FCCOB8 = commonCommandObject->DataByte0;
	FTFE->FCCOB9 = commonCommandObject->DataByte1;
	FTFE->FCCOBA = commonCommandObject->DataByte2;
	FTFE->FCCOBB = commonCommandObject->DataByte3;
	FTFE->FCCOB4 = commonCommandObject->DataByte4;
	FTFE->FCCOB5 = commonCommandObject->DataByte5;
	FTFE->FCCOB6 = commonCommandObject->DataByte6;
	FTFE->FCCOB7 = commonCommandObject->DataByte7;

	FTFE->FSTAT = FTFE_FSTAT_CCIF_MASK;

	return true;
}

/*! @brief Enables the Flash module.
 *
 *  @return bool - TRUE if the Flash was setup successfully.
 */
bool Flash_Init(void)
{
	return true;
}

/*! @brief Allocates space for a non-volatile variable in the Flash memory.
 *
 *  @param variable is the address of a pointer to a variable that is to be allocated space in Flash memory.
 *         The pointer will be allocated to a relevant address:
 *         If the variable is a byte, then any address.
 *         If the variable is a half-word, then an even address.
 *         If the variable is a word, then an address divisible by 4.
 *         This allows the resulting variable to be used with the relevant Flash_Write function which assumes a certain memory address.
 *         e.g. a 16-bit variable will be on an even address
 *  @param size The size, in bytes, of the variable that is to be allocated space in the Flash memory. Valid values are 1, 2 and 4.
 *  @return bool - TRUE if the variable was allocated space in the Flash memory.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_AllocateVar(volatile void** variable, const uint8_t size)
{

}

/*! @brief Writes a 32-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{

}

/*! @brief Writes a 16-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{

}

/*! @brief Writes an 8-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{

}

/*! @brief Erases the entire Flash sector.
 *
 *  @return bool - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Erase(void)
{

}

