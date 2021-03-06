/*!
**  @addtogroup FIFO_module FIFO module documentation
**  @{
*/
/* MODULE FIFO */
/*! @file FIFO.c
 *
 *  @brief Routines to for a FIFO buffer.
 *
 *  This contains the functionality for a byte-wide FIFO.
 *
 *  @author Uldis Bagley and Prashant Shrestha
 *  @date 2020-03-18
 */

#include "FIFO.h"
#include "Critical\critical.h"
#include "fsl_common.h"
#include "fsl_port.h"


bool FIFO_Init(TFIFO* const fifo)
{
	EnterCritical();
	// Initialise variables to 0
	fifo->Start = fifo->End = fifo->NbBytes = 0;

	ExitCritical();
	return true;
}

bool FIFO_Put(TFIFO* const fifo, const uint8_t data)
{
	EnterCritical();
	// Check that FIFO buffer isn't full
	if (fifo->NbBytes == FIFO_SIZE)
	{
		ExitCritical();
		return false;
	}
	else
	{
		fifo->Buffer[fifo->End] = data; // write data into buffer

		if (fifo->End == FIFO_SIZE-1) // Check if End is in the buffer's last location. If it is wrap it around to the beginning
			fifo->End = 0;
		else
			fifo->End++; // increment End to the next buffer location

		fifo->NbBytes++; // Since we have added a byte to the buffer

		ExitCritical();
		return true;
	}
}

bool FIFO_Get(TFIFO* const fifo, uint8_t* const dataPtr)
{
	EnterCritical();
	// Check that FIFO buffer isn't empty
	if (fifo->NbBytes == 0)
	{
		ExitCritical();
		return false;
	}
	else
	{
		*dataPtr = fifo->Buffer[fifo->Start]; // write the buffer's contents at Start into the dataPtr

		if (fifo->Start == FIFO_SIZE-1) // Check if Start is in the buffer's last location. If it is wrap it around to the beginning
			fifo->Start = 0;
		else
			fifo->Start++; // increment End to the next buffer location

		fifo->NbBytes--; // Since we have added a byte to the buffer

		ExitCritical();
		return true;
	}
}

/* END FIFO */
/*!
** @}
*/
