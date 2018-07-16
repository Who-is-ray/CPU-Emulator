#pragma once
#include"Memory.h"	//header that define memory class

#define SIZE_OF_CACHE 32	//size of cache

struct dirty_byte	//dirty byte and access
{
	unsigned char dirty_bit : 1;	//dirty bit indicated the data has been changed or not to maintain consistance
	unsigned char reserved : 2;	//unused 2 bits
	unsigned char age : 5;	//age bits indicated the most recently used data
};

union dirty
{
	unsigned char byte = 0;
	struct dirty_byte dirty_byte;
};

struct cache_line
{
	unsigned short address = 0;	//primary memory address with initial value of zero
	unsigned short content = 0;	//contents of the primary memory address with initial value of zero
	dirty dirty;	//dirty bit with initial value of zero, and age value with initial value of zero
};

class Cache_Memory
{
public:
	Cache_Memory(Memory& mem, unsigned int& clock) : m_memory(mem), m_clock(clock) {}	//Constructor, initialize the reference of m_memory
	~Cache_Memory() {}	//Destructor

	void cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw = WORD);	//cache function, will be called by CPU when access data in cache

	struct cache_line cache_mem[SIZE_OF_CACHE];	//cache memory cont

private:
	Memory& m_memory;	//the reference of memory
	unsigned int& m_clock;	//system clock
};