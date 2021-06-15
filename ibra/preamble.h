#ifndef PREAMBLE_H
#define PREAMBLE_H

#include <stdint.h>

/**********************************************************************\
|* Enums and Typedefs
\**********************************************************************/
typedef enum
	{
	TYPE_NONE	= 0,
	TYPE_UPDATE,
	TYPE_SAMPLE
	} PreambleType;

struct Preamble
	{
	uint16_t order;
	uint16_t offset;
	uint32_t extent;
	uint16_t type;
	uint16_t flags;

	/**************************************************************************\
	|* Constructor just to set common things
	\**************************************************************************/
	Preamble(void)
		{
		order	= 0xAA55;
		offset	= sizeof(Preamble);
		extent	= 0;
		type	= 0;
		flags	= 0;
		}

	/**************************************************************************\
	|* Determine if we're swapped compared to the source
	\**************************************************************************/
	bool isByteSwapped()
		{
		return (order == 0xAA55) ? false : true;
		}
	};

#endif // PREAMBLE_H
