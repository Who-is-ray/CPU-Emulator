#include "Cache_Memory.h"

#define READ_HIGH_BYTE(X)	X>>8
#define READ_LOW_BYTE(X)	(X&0xff)
#define TO_HIGH_BYTE(X)		X<<8
#define TO_LOW_BYTE(X)		(X&0xff)
#define GET_MEMORY_ADDR(X)	(X&0xfffe)
#define GET_DIRECT_CACHE_ADDR(X)	(X&2)>>1

#define DIRECT_MAPPING
#define ASSOCIATIVE

#ifdef DIRECT_MAPPING
//cache function for Direct mapping
void Cache_Memory::cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw)
{
	unsigned short addr = MAR >> 1;	//because cache content is word size, each content includes two memory byte: mem[n] and mem[n+1], n is even, so addr ignored even or odd difference
	unsigned char c_addr = GET_DIRECT_CACHE_ADDR(MAR);	//get cache address
	if (rw == READ)	//read
	{
		if (cache_mem[c_addr].address == addr)	//if hit
		{
			if (bw == BYTE)	//access byte size data
				MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(cache_mem[c_addr].content) : READ_LOW_BYTE(cache_mem[c_addr].content);	//assign high or low byte data to MDR
			else	//access word size data
				MDR = cache_mem[c_addr].content;	//assign data to MDR
		}
		else	//if miss
		{
			if (cache_mem[c_addr].dirty.dirty_byte.dirty_bit)	//if dirty bit set
			{
				m_memory.bus((cache_mem[c_addr].address << 1), cache_mem[c_addr].content, WRITE);	//write back to memory
				cache_mem[c_addr].dirty.dirty_byte.dirty_bit = 0;	//clear dirty bit
			}
			if (bw == BYTE)	//access byte size data
			{
				m_memory.bus(GET_MEMORY_ADDR(MAR), MDR, READ);	//get data from primary memroy
				cache_mem[c_addr].content = MDR;	//assign data to cache
				cache_mem[c_addr].address = addr;	//assign address to cache
				MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(MDR) : READ_LOW_BYTE(MDR);	//assign high or low byte data to MDR
			}
			else	//access word size data
			{
				m_memory.bus(MAR, MDR, READ);	//get data from primary memroy
				cache_mem[c_addr].content = MDR;	//assign data to cache
				cache_mem[c_addr].address = addr;	//assign address to cache
			}
		}
	}
	else	//write
	{
		if (cache_mem[c_addr].address == addr)	//if hit
		{
			if (bw == BYTE)	//access byte size data
			{

			}
			else	//access word size data
				cache_mem[c_addr].content = MDR;	//assign data to cache
		}
	}
}
#endif // DIRECT_MAPPING