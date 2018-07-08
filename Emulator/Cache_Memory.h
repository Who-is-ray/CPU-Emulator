#pragma once

#define SIZE_OF_CACHE 16	//size of cache

class Memory;

struct cache_line
{
	unsigned short address = 0;	//primary memory address with initial value of zero
	unsigned short content = 0;	//contents of the primary memory address with initial value of zero
	unsigned char  dirty = 0;	//dirty bit to maintain consistance with initial value of zero
};

class Cache_Memory
{
public:
	Cache_Memory(Memory& mem, unsigned int& clock) : m_memory(mem), m_clock(clock) {}	//Constructor, initialize the reference of m_memory
	~Cache_Memory() {}	//Destructor

	void cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw = WORD);	//cache function, will be called by CPU when access data in cache

private:
	struct cache_line cache_mem[SIZE_OF_CACHE];	//cache memory cont
	Memory& m_memory;	//the reference of memory
	unsigned int& m_clock;	//system clock
};

