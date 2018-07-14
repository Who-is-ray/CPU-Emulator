#include "Cache_Memory.h"

#define DIRECT_MAPPING

#ifdef DIRECT_MAPPING
//cache function for Direct mapping
void Cache_Memory::cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw)
{
	bool hit = false;
	if (rw == READ)	//read
	{
		for (size_t i = 0; i < SIZE_OF_CACHE; i++)
		{
			if (cache_mem->address == MAR)
			{

				hit = true;	//set hit flag
			}
		}
	}
	else	//write
	{

	}
}
#endif // DIRECT_MAPPING