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

// Private functions:
static bool LaunchCommand(FCCOB_t* commonCommandObject);
static bool WritePhrase(const uint32_t address, const uint64union_t phrase);
static bool EraseSector(const uint32_t address);
static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);

// I think FCCOB is RAM buffer that allows us to change 1 or more bytes of information while the Flash sector is being erased. Then we write the FCCOB data with the change into the Flash

typedef struct
{


}FCCOB_t;

const uint8_t HALF_WORD_ALIGNED = 0;
const uint8_t WORD_ALIGNED = 0;
const uint8_t PHRASE_ALIGNED = 0;

bool Flash_Init(void)
{
	// FSTAT_CCIF flag will be used during erasing operations
	// Nothing to initialize
	return true;
}

static bool LaunchCommand(FCCOB_t* commonCommandObject)
{
	// if((FSTAT_ACCERR && FSTAT_FPVIOL == 0) && (FSTAT_CCIF == 1)) then run FCCOB changes
}


static bool WritePhrase(const uint32_t address, const uint64union_t phrase)
{

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
	uint8_t allocation[7]; // <---- THIS will become address placements in the FCCOB struct
	int i;
	const FLASH_START = 0;
	const FLASH_END = FLASH_DATA_END - FLASH_DATA_START;

	switch (size) // switch case with respect to the size of variable in bytes
	{
		case 1:
			for (i = FLASH_START; i < (FLASH_END + 1); i+= size)
			{
				// Check if location of 'i' is empty
				if (allocation[i] == 0) // if place is empty we fill with our value
				{
					allocation[i] = *variable; // allocate address for data to reside
					return true;
				}
				else if (allocation[i] != 0) // if allocation is full we increment
					break;
				else // if all places are full we return false
					return false;
			}

		case 2:
			for (i = FLASH_START; i < (FLASH_END + 1); i+= size)
			{
				// Check if location of 'i' is empty
				if (allocation[i] == 0) // if place is empty we fill with our value
				{
					allocation[i] = *variable; // allocate address for data to reside
					return true;
				}
				else if (allocation[i] != 0) // if allocation is full we increment
					break;
				else // if all places are full we return false
					return false;
			}

		case 4:
			for (i = FLASH_START; i < (FLASH_END + 1); i+= size)
			{
				// Check if location of 'i' is empty
				if (allocation[i] == 0) // if place is empty we fill with our value
				{
					allocation[i] = *variable; // allocate address for data to reside
					return true;
				}
				else if (allocation[i] != 0) // if allocation is full we increment
					break;
				else // if all places are full we return false
					return false;
			}

		default:
			return false;
	}
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
