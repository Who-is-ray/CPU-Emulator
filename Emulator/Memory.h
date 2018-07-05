#pragma once

#define SIZE_OF_BYTE_MEMORY	(1<<16)	//size of byte memory array
#define	SIZE_OF_WORD_MEMORY (1<<15)	//size of word memory array
#define CSR_OF	(1<<3)	//control/status register bit Overrun
#define CSR_DBA	(1<<2)	//control/status register bit DBA
#define CSR_IO	(1<<1)	//control/status register bit I/O
#define CSR_IE	1		//control/status register bit IE
#define CSR_SET_INPUT 2	//value to set CSR input
#define CSR_SET_OUTPUT 0	//value to set CSR output

enum SIZE { BYTE, WORD };	//enum of size, byte = 0, word = 1
enum ACTION { READ, WRITE };	//enum of action, read = 0, write = 1

union memory	//memory union	??put in cpp file?
{
	unsigned char byte_mem[SIZE_OF_BYTE_MEMORY]{0};	//memory array of byte size elements
	unsigned short word_mem[SIZE_OF_WORD_MEMORY];	//memory array of word size elements
};

class Memory	//memory class
{
public:
	Memory(unsigned int& clock) :m_clock(clock) {}	//consturctor, set reference of m_clock
	~Memory() {}	//destructor
	void bus(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw = WORD);	//bus function
	union memory m_memory;	//memory union
	void update_CSR(ACTION rw, SIZE bw, unsigned short address);	//update CSR after accessed CSR data
private:
	unsigned int& m_clock;	//system clock
};
