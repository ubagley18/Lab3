/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-04-10
 */

#include "Flash.h"
#include "fsl_common.h"
#include "fsl_port.h"


#define NB_ADDRESS_REG 3
#define NB_DATA_REG  8

typedef struct
{
  uint8_t command;
  union
  {
    uint32_t combined;
    struct
    {
      uint8_t flashAddress0;
      uint8_t flashAddress1;
      uint8_t flashAddress2;
      uint8_t flashAddress3;
    } separate;
  } address;  // The first 4 FCCOB registers (0-3)
  union
  {
    uint64_t combined;
    struct
    {
      uint8_t dataByte0;
      uint8_t dataByte1;
      uint8_t dataByte2;
      uint8_t dataByte3;
      uint8_t dataByte4;
      uint8_t dataByte5;
      uint8_t dataByte6;
      uint8_t dataByte7;
    } separate;
  } data;  // The last 8 FCCOB registers (4-B)
} FCCOB_t;

const uint8_t HALF_WORD_ALIGNED = 0;
const uint8_t PHRASE_ALIGNED = 0;
const uint8_t WORD_ALIGNED = 0;
const uint8_t WRITE = 0x07;
FCCOB_t* FCCOB;


// Private functions:
static bool LaunchCommand(FCCOB_t* commonCommandObject);
static bool WritePhrase(const uint32_t address, const uint64union_t phrase);
static bool EraseSector(const uint32_t address);
static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);

// I think FCCOB is RAM buffer that allows us to change 1 or more bytes of information while the Flash sector is being erased. Then we write the FCCOB data with the change into the Flash



bool Flash_Init(void)
{
	// FSTAT_CCIF flag will be used during erasing operations
	// Nothing to initialize
	return true;
}


static bool EraseSector(const uint32_t address)
{

}


static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase)
{

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
	// Allocate variable to space in flash memory for the pointer address
	// uint8_t allocation[7]; // <---- THIS will become address placements in the FCCOB struct
	int i;
	const uint8_t FLASH_START = 0;
	const uint8_t FLASH_END = FLASH_DATA_END - FLASH_DATA_START;

	for (i = FLASH_START; i < (FLASH_END + 1); i+= size)
	{
		// Check if location of 'i' is empty
		if (FCCOB->data[i] == 0) // if place is empty we fill with our value
		{
			FCCOB->data[i] = *variable; // allocate address for data to reside
			return true;
		}
		else if (FCCOB->data[i] != 0) // if allocation is full we increment
			continue;
		else // if all places are full we return false
			return false;
	}
}


static bool LaunchCommand(FCCOB_t* commonCommandObject)
{
	// write parameters into registers with big endian notation
  FTFE->FCCOB0 = commonCommandObject->command;
  FTFE->FCCOB1 = commonCommandObject->address.separate.flashAddress2;
  FTFE->FCCOB2 = commonCommandObject->address.separate.flashAddress1;
  FTFE->FCCOB3 = commonCommandObject->address.separate.flashAddress0;
  FTFE->FCCOB4 = commonCommandObject->data.separate.dataByte7;
  FTFE->FCCOB5 = commonCommandObject->data.separate.dataByte6;
  FTFE->FCCOB6 = commonCommandObject->data.separate.dataByte5;
  FTFE->FCCOB7 = commonCommandObject->data.separate.dataByte4;
  FTFE->FCCOB8 = commonCommandObject->data.separate.dataByte3;
  FTFE->FCCOB9 = commonCommandObject->data.separate.dataByte2;
  FTFE->FCCOBA = commonCommandObject->data.separate.dataByte1;
  FTFE->FCCOBB = commonCommandObject->data.separate.dataByte0;

	FTFE->FSTAT = FTFE_FSTAT_CCIF_MASK; // clear the CCIF to launch the command
	// WHAT DOES THE DIAGRAM ON P702 MEAN BY MORE PARAMETERS?

}


static bool WritePhrase(const uint32_t address, const uint64union_t phrase)
{
	// Wait for the CCIF to be set so a command can be executed
	while (FTFE->FSTAT != FTFE_FSTAT_CCIF_MASK);

	// clear protection violation and access error flag if set
	if ((FTFE->FSTAT & FTFE_FSTAT_FPVIOL_MASK) == FTFE_FSTAT_FPVIOL_MASK)
		FTFE->FSTAT = FTFE_FSTAT_FPVIOL_MASK;
	if ((FTFE->FSTAT & FTFE_FSTAT_ACCERR_MASK) == FTFE_FSTAT_ACCERR_MASK)
			FTFE->FSTAT = FTFE_FSTAT_ACCERR_MASK;

	// Load FCCOB struct with values

	FCCOB->command = WRITE;
	FCCOB->address.combined = address;
	FCCOB->data.combined = phrase.l;

	return LaunchCommand(FCCOB);
}


bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{

	//if ((address < FLASH_DATA_START) || (address > FLASH_DATA_END)
	uint32_t* phraseAddress;
	uint64union_t phraseData;

	phraseAddress = (uint32_t*)address;

	if ((*phraseAddress < FLASH_DATA_START) || (*phraseAddress > FLASH_DATA_END))
		return false;
	else if (*phraseAddress % 8 == PHRASE_ALIGNED)
	{
		phraseData.s.Lo = data;
		phraseData.s.Hi = _FW(address + 4); // read the existing word from the address of the register 4 bytes ahead
	}
	else
	{
		phraseData.s.Lo = _FW(address - 4); // read the existing word from the address of the register 4 bytes behind
		phraseData.s.Hi = data;
	}

	return WritePhrase(*phraseAddress, phraseData);
}


bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
	/*and 32-bit unsigned words on an address evenly divisible by 4.
		The HAL should support the allocation of Flash memory at the user’s request,
		up to a maximum of 8 bytes total, i.e. we are restricting Flash memory allocation to one phrase.*/
	uint32_t* wordAddress;
	uint32union_t wordData;

	wordAddress = (uint32_t*)address;

	if (*wordAddress % 4 == WORD_ALIGNED)
	{
		wordData.s.Lo = data;
		wordData.s.Hi = _FH(address + 2); // read the existing half-word from the address of the register 2 bytes ahead
	}
	else
	{
		wordData.s.Lo = _FH(address - 2); // read the existing half-word from the address of the register 2 bytes behind
		wordData.s.Hi = data;
	}

	return Flash_Write32(wordAddress, wordData.l);
}


bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
	uint16_t* halfWordAddress;
	uint16union_t halfWordData;

	halfWordAddress = (uint16_t*)address;

	if (*halfWordAddress % 2 == HALF_WORD_ALIGNED)
	{
		halfWordData.s.Lo = data; // read data onto the existing address
		halfWordData.s.Hi = _FB(address + 1); // read the existing byte at the address of the next register
	}
	else
	{
		halfWordData.s.Lo = _FB(address - 1); // read the existing byte at the address of the previous register
		halfWordData.s.Hi = data;
	}

	return Flash_Write16(halfWordAddress, halfWordData.l);
}



bool Flash_Erase(void)
{

}
