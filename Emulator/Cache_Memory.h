#pragma once
#include"Memory.h"	//header that define memory class

#define NUM_OF_CACHELINE 32	//size of cache for cache line
#define NUM_OF_PAGE	8		//number of page in cache
#define LINE_PER_PAGE 4		//number of cache line per page

//#define DIRECT_MAPPING_CACHE
#define ASSOCIATIVE_CACHE
//#define HYBRID_CACHE
//#define WRITE_BACK
#define WRITE_THROUGH

struct dirty_byte	//dirty byte and access
{
	unsigned char dirty_bit : 1;	//dirty bit indicated the data has been changed or not to maintain consistance
	unsigned char reserved : 2;	//unused 2 bits
#ifdef HYBRID_CACHE
	unsigned char page_addr : 3;	//address of page
	unsigned char age : 2;	//age bits indicated the most recently used data
#else
	unsigned char age : 5;	//age bits indicated the most recently used data
#endif
};

union dirty	//define dirty_byte as byte size
{
	unsigned char byte = 0;	//byte size with initial value of zero
	struct dirty_byte dirty_byte;
};

struct cache_line	//structure of cache line
{
	unsigned short address = 0;	//primary memory address with initial value of zero
	unsigned short content = 0;	//contents of the primary memory address with initial value of zero
	dirty dirty;	//dirty bit with initial value of zero, and age value with initial value of zero
};

struct page		//struct of page
{
	struct cache_line cache_line[LINE_PER_PAGE];
};

class Cache_Memory	//memory of class
{
public:
	Cache_Memory(Memory& mem, unsigned int& clock) : m_memory(mem), m_clock(clock) {}	//Constructor, initialize the reference of m_memory
	~Cache_Memory() {}	//Destructor

	void cache(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw = WORD);	//cache function, will be called by CPU when access data in cache

#ifdef HYBRID_CACHE
	struct page cache_mem[NUM_OF_PAGE];	//cache memory cont
#else
	struct cache_line cache_mem[NUM_OF_CACHELINE];	//cache memory cont
#endif // HYBRID_CACHE

private:
	Memory& m_memory;	//the reference of memory
	unsigned int& m_clock;	//system clock
};