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
#define BYTE 1
#define HALF_WORD 2
#define WORD 4

static uint8_t FlashMemory = 0xFF;


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
  } data;  // FCCOB registers 4-B
} FCCOB_t;

const uint8_t HALF_WORD_ALIGNED = 0;
const uint8_t PHRASE_ALIGNED = 0;
const uint8_t WORD_ALIGNED = 0;
const uint8_t WRITE = 0x07;
const uint8_t ERASE = 0x09;
FCCOB_t* FCCOB;


// Private functions:
static bool LaunchCommand(FCCOB_t* commonCommandObject);
static bool WritePhrase(const uint32_t address, const uint64union_t phrase);
static bool EraseSector(const uint32_t address);
static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);

// I think FCCOB is RAM buffer that allows us to change 1 or more bytes of information while the Flash sector is being erased. Then we write the FCCOB data with the change into the Flash



bool Flash_Init(void)
{
	// Nothing to initialize
	return true;
}


static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase)
{
	//first erase flash, then write entire phrase
	return Flash_Erase() && WritePhrase(address, phrase);
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

	static bool FlashSectorsAllocated[8] = { false };
	uint32_t addressForVariable;

	if (size == 1 || size == 2 || size == 4) // check to see that only accepted data is inputted
	{
		for (int i = 0; i < 8; i += size)
		{
			if (FlashSectorsAllocated[i] == 0x00) //check to see if the sector is available
			{
				addressForVariable = FLASH_DATA_START + i; // calculate the address that will be assigned
				for (int x = 0; x < size; x++)
				{
					FlashSectorsAllocated[i] = 0x01; // block out the rest of the sectors that are no longer usable
				}
				*variable = (void*)addressForVariable; //assign the pointer variable to the allocated flash address
				return true;
			}
		}
	}
	else
	{
		return false;
	}
	return true;

//	  static uint8_t memoryAlloc; //memory mask to allocate memory position
//	  static int variableAddress; //temporary variable address
//
//
//	  //check the size of the variable
//	  switch (size)
//	  {
//	    //assign potential memory depending on the size
//	    case 1:
//	      memoryAlloc = 0x01; // 0000 0001
//	      break;
//	    case 2:
//	      memoryAlloc = 0x03; // 0000 0011
//	      break;
//	    case 4:
//	      memoryAlloc = 0x0F; // 0000 1111
//	      break;
//	  }
//
//
//	  //loop through the address incrementing by variable size to find an unused memory
//	  for (variableAddress = FLASH_DATA_START; variableAddress <= FLASH_DATA_END; variableAddress += size)
//	  {
//	    //check if the flash is empty
//	    if (memoryAlloc == (FlashMemory & memoryAlloc))
//	    {
//	      //write the selected address to the address of the memory
//	      *variable = (void *) variableAddress;
//	      //change the current memory as used
//	      FlashMemory = (FlashMemory ^ memoryAlloc);
//	      return true;
//	    }
//	    //shift down the memoryAlloc until Flash Memory is empty
//	    memoryAlloc = memoryAlloc << size;
//	  }
//	  return false;


//	// Allocate variable to space in flash memory for the pointer address
//	uint32_t* possibleAddress; // possible address to allocate variable
//	uint32_t access;
//
//
//	switch (size)
//	{
//		case BYTE:
//			access = _FB(possibleAddress);
//			break;
//		case HALF_WORD:
//			access = _FH(possibleAddress);
//			break;
//		case WORD:
//			access = _FW(possibleAddress);
//			break;
//	}
//
//	for (*possibleAddress = FLASH_DATA_START; *possibleAddress < (FLASH_DATA_END + 1); *possibleAddress+= size)
//	{
//		// Check if location of 'i' is empty
//		if (access == 0) // if place is empty we fill with our value
//		{
//			*variable = (void *)access; // allocate address for data to reside
//			return true;
//		}
//		else if (access != 0) // if allocation is full we increment
//			continue;
//		else // if all places are full we return false
//			return false;
//	}
//	return true;
}


static bool LaunchCommand(FCCOB_t* commonCommandObject)
{
	// write parameters into registers with big endian notation
  FTFE->FCCOB0 = commonCommandObject->command;
  FTFE->FCCOB1 = commonCommandObject->address.separate.flashAddress2;
  FTFE->FCCOB2 = commonCommandObject->address.separate.flashAddress1;
  FTFE->FCCOB3 = commonCommandObject->address.separate.flashAddress0;
  FTFE->FCCOB8 = commonCommandObject->data.separate.dataByte7;
  FTFE->FCCOB9 = commonCommandObject->data.separate.dataByte6;
  FTFE->FCCOBA = commonCommandObject->data.separate.dataByte5;
  FTFE->FCCOBB = commonCommandObject->data.separate.dataByte4;
  FTFE->FCCOB4 = commonCommandObject->data.separate.dataByte3;
  FTFE->FCCOB5 = commonCommandObject->data.separate.dataByte2;
  FTFE->FCCOB6 = commonCommandObject->data.separate.dataByte1;
  FTFE->FCCOB7 = commonCommandObject->data.separate.dataByte0;

	FTFE->FSTAT = FTFE_FSTAT_CCIF_MASK; // clear the CCIF to launch the command
	// WHAT DOES THE DIAGRAM ON P702 MEAN BY MORE PARAMETERS?

	return true;
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

	return ModifyPhrase(*phraseAddress, phraseData);
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
	uint32_t sectorErase = FLASH_DATA_START;
	return EraseSector(sectorErase);
}

static bool EraseSector(const uint32_t address)
{
	FCCOB->command = ERASE;
	//assign address to be erased
	FCCOB->address.combined = address;
	return LaunchCommand(FCCOB);
}
