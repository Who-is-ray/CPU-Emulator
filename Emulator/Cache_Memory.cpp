#include "Cache_Memory.h"

#define CLEAR_HIGH_BYTE(X)	(X&0xff)
#define CLEAR_LOW_BYTE(X)	(X&0xff00)
#define READ_HIGH_BYTE(X)	(X>>8)
#define TO_HIGH_BYTE(X)		(X<<8)
#define GET_MEMORY_ADDR(X)	(X&0xfffe)
#define GET_DIRECT_CACHE_ADDR(X)	((X&0x3e)>>1)
#define DEVICE_CSR	16
#define DEVICE_VECTOR	0xffc0
#define HIGHEST_AGE	0x1f


//#define DIRECT_MAPPING_CACHE
#define ASSOCIATIVE_CACHE
//#define WRITE_BACK
#define WRITE_THROUGH

#ifdef DIRECT_MAPPING_CACHE
void Cache_Memory::cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw)	//cache function for Direct mapping
{
	if (MAR > DEVICE_CSR)	//if not access any device CSR and device vector
	{
		unsigned short addr = MAR >> 1;	//because cache content is word size, each content includes two memory byte: mem[n] and mem[n+1], n is even, so addr ignored even or odd difference
		unsigned unsigned int c_addr = GET_DIRECT_CACHE_ADDR(MAR);	//get cache address
		m_clock += 2;
		if (rw == READ)	//read
		{
			if (cache_mem[c_addr].address == addr)	//if hit
				if (bw == BYTE)	//access byte size data
					MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(cache_mem[c_addr].content) : CLEAR_HIGH_BYTE(cache_mem[c_addr].content);	//assign high or low byte data to MDR
				else	//access word size data
					MDR = cache_mem[c_addr].content;	//assign data to MDR
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
					MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(MDR) : CLEAR_HIGH_BYTE(MDR);	//assign high or low byte data to MDR
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
#ifdef WRITE_BACK
			if (cache_mem[c_addr].address == addr)	//if hit
			{
				if (bw == BYTE)	//access byte size data
					cache_mem[c_addr].content = ((MAR & 1) == 1) ?
					(CLEAR_HIGH_BYTE(cache_mem[c_addr].content) + TO_HIGH_BYTE(MDR)) :
					(CLEAR_LOW_BYTE(cache_mem[c_addr].content) + CLEAR_HIGH_BYTE(MDR));	// assign high/low byte data to cache
				else	//access word size data
					cache_mem[c_addr].content = MDR;	//assign data to cache
			}
			else	//miss
			{
				if (cache_mem[c_addr].dirty.dirty_byte.dirty_bit)	//if dirty bit set
					m_memory.bus((cache_mem[c_addr].address << 1), cache_mem[c_addr].content, WRITE);	//write back to memory
				if (bw == BYTE)	//if access byte size memory
				{
					m_memory.bus(GET_MEMORY_ADDR(MAR), cache_mem[c_addr].content, READ);	//load content from memory to cache
					cache_mem[c_addr].content = ((MAR & 1) == 1) ?
						(CLEAR_HIGH_BYTE(cache_mem[c_addr].content) + TO_HIGH_BYTE(MDR)) :
						(CLEAR_LOW_BYTE(cache_mem[c_addr].content) + CLEAR_HIGH_BYTE(MDR));	// assign high/low byte data to cache
				}
				else	//if access word size memory
					cache_mem[c_addr].content = MDR;	//write data to cache
				cache_mem[c_addr].address = addr;	//assign correct address to cache
			}
			cache_mem[c_addr].dirty.dirty_byte.dirty_bit = 1;	//set dirty bit
#endif // WRITE_BACK
#ifdef WRITE_THROUGH
			m_memory.bus(MAR, MDR, WRITE, bw);	//load content from memory to cache
			if (bw == BYTE)	//if access byte size memory
				m_memory.bus(GET_MEMORY_ADDR(MAR), cache_mem[c_addr].content, READ);	//keep word content consistent
			else	//if access word size memory
				cache_mem[c_addr].content = MDR;	//write data to cache
			cache_mem[c_addr].address = addr;	//assign correct address to cache
#endif // WRITE_THROUGH
		}
	}
	else	//if access device CSR or device vector
		m_memory.bus(MAR, MDR, rw, bw);	//call bus
}

#endif // DIRECT_MAPPING_CACHE

#ifdef ASSOCIATIVE_CACHE
void Cache_Memory::cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw)	//cache function for Direct mapping
{
	if (MAR > DEVICE_CSR)	//if not access any device CSR and device vector
	{
		unsigned short addr = MAR >> 1;	//because cache content is word size, each content includes two memory byte: mem[n] and mem[n+1], n is even, so addr ignored even or odd difference
		bool hit = false;	//hit flag
		m_clock += 2;
		for (size_t c_addr = 0; c_addr < SIZE_OF_CACHE; c_addr++)
		{
			if (cache_mem[c_addr].address == addr)	//if hit
			{
				unsigned char age = cache_mem[c_addr].dirty.dirty_byte.age;	//previous cache line's age

				if (rw == READ)	//read
					if (bw == BYTE)	//access byte size data
						MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(cache_mem[c_addr].content) : CLEAR_HIGH_BYTE(cache_mem[c_addr].content);	//assign high or low byte data to MDR
					else	//access word size data
						MDR = cache_mem[c_addr].content;	//assign data to MDR
				else	//write
				{
#ifdef WRITE_BACK
					if (bw == BYTE)	//access byte size data
						cache_mem[c_addr].content = ((MAR & 1) == 1) ?
						(CLEAR_HIGH_BYTE(cache_mem[c_addr].content) + TO_HIGH_BYTE(MDR)) :
						(CLEAR_LOW_BYTE(cache_mem[c_addr].content) + CLEAR_HIGH_BYTE(MDR));	// assign high/low byte data to cache
					else	//access word size data
						cache_mem[c_addr].content = MDR;	//assign data to cache
					cache_mem[c_addr].dirty.dirty_byte.dirty_bit = 1;	//set dirty bit
#endif // WRITE_BACK
#ifdef WRITE_THROUGH
					m_memory.bus(MAR, MDR, WRITE, bw);	//load content from memory to cache
					if (bw == BYTE)	//if access byte size memory
						m_memory.bus(GET_MEMORY_ADDR(MAR), cache_mem[c_addr].content, READ);	//keep word content consistent
					else	//if access word size memory
						cache_mem[c_addr].content = MDR;	//write data to cache
#endif // WRITE_THROUGH
				}
				hit = true;

				for (size_t i = 0; i < SIZE_OF_CACHE; i++)
					if (cache_mem[i].dirty.dirty_byte.age > age)
						cache_mem[i].dirty.dirty_byte.age -= 1;	//update age
				cache_mem[c_addr].dirty.dirty_byte.age = HIGHEST_AGE;	//set latest cache to highest age value
				break;
			}
		}
		if (!hit)	//if miss
		{
			unsigned int c_addr;
			for (size_t i = 0; i < SIZE_OF_CACHE; i++)
			{
				if (cache_mem[i].dirty.dirty_byte.age == 0)	//get the least recent cache address
					c_addr = i;
				else
					cache_mem[i].dirty.dirty_byte.age -= 1;	//update cache's age
			}
			if (cache_mem[c_addr].dirty.dirty_byte.dirty_bit)	//if dirty bit set
				m_memory.bus((cache_mem[c_addr].address << 1), cache_mem[c_addr].content, WRITE);	//write back to memory
			if (rw == READ)	//read
			{
				cache_mem[c_addr].dirty.dirty_byte.dirty_bit = 0;	//clear dirty bit
				if (bw == BYTE)	//access byte size data
				{
					m_memory.bus(GET_MEMORY_ADDR(MAR), MDR, READ);	//get data from primary memroy
					cache_mem[c_addr].content = MDR;	//assign data to cache
					cache_mem[c_addr].address = addr;	//assign address to cache
					MDR = ((MAR & 1) == 1) ? READ_HIGH_BYTE(MDR) : CLEAR_HIGH_BYTE(MDR);	//assign high or low byte data to MDR
				}
				else	//access word size data
				{
					m_memory.bus(MAR, MDR, READ);	//get data from primary memroy
					cache_mem[c_addr].content = MDR;	//assign data to cache
				}
			}
			else	//write
			{
#ifdef WRITE_BACK
				if (bw == BYTE)	//if access byte size memory
				{
					m_memory.bus(GET_MEMORY_ADDR(MAR), cache_mem[c_addr].content, READ);	//load content from memory to cache
					cache_mem[c_addr].content = ((MAR & 1) == 1) ?
						(CLEAR_HIGH_BYTE(cache_mem[c_addr].content) + TO_HIGH_BYTE(MDR)) :
						(CLEAR_LOW_BYTE(cache_mem[c_addr].content) + CLEAR_HIGH_BYTE(MDR));	// assign high/low byte data to cache
				}
				else	//if access word size memory
					cache_mem[c_addr].content = MDR;	//write data to cache
				cache_mem[c_addr].dirty.dirty_byte.dirty_bit = 1;	//set dirty bit
#endif // WRITE_BACK
#ifdef WRITE_THROUGH
				m_memory.bus(MAR, MDR, WRITE, bw);	//load content from memory to cache
				if (bw == BYTE)	//if access byte size memory
					m_memory.bus(GET_MEMORY_ADDR(MAR), cache_mem[c_addr].content, READ);	//keep word content consistent
				else	//if access word size memory
					cache_mem[c_addr].content = MDR;	//write data to cache
#endif // WRITE_THROUGH
			}
			cache_mem[c_addr].address = addr;	//assign correct address to cache
			cache_mem[c_addr].dirty.dirty_byte.age = HIGHEST_AGE;	//set latest cache to highest age value
		}
	}
	else	//if access device CSR or device vector
		m_memory.bus(MAR, MDR, rw, bw);	//call bus
}
#endif // ASSOCIATIVE_CACHE