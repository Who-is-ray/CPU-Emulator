#pragma once

#define SIZE_OF_BYTE_MEMORY	1<<16	//size of byte memory array
#define	SIZE_OF_WORD_MEMORY 1<<15	//size of word memory array

enum SIZE { BYTE, WORD };	//enum of size, byte = 0, word = 1
enum ACTION { READ, WRITE };	//enum of action, read = 0, write = 1

union memory	//memory union
{
	unsigned char byte_mem[SIZE_OF_BYTE_MEMORY];	//memory array of byte size elements
	unsigned short word_mem[SIZE_OF_WORD_MEMORY];	//memory array of word size elements
};

class Memory	//memory class
{
public:
	Memory() {}	//consturctor
	~Memory() {}	//destructor
	void bus(unsigned short MAR, unsigned short& MDR, enum ACTION rw, enum SIZE bw = WORD);	//bus function
	union memory m_memory;	//memory union
};
