#include "Memory.h"	//header that define memory class

/*
	bus function
	MAR - memory address
	MDR - reference to data to read/write
	rw	- read|write
	bw	- byte|word
*/
void Memory::bus(unsigned short MAR, unsigned short& MDR, enum ACTION rw, enum SIZE bw)
{	//if address < 16		??byte read reset DBA?
	if (MAR < 16)
	{

	}
	else
	{
		if (rw == READ)
			MDR = (bw == WORD) ? m_memory.word_mem[MAR / 2] : m_memory.byte_mem[MAR];	//read word/byte data to MDR
		else	//write
		{
			if (bw == WORD)
				m_memory.word_mem[MAR / 2] = MDR;	//write word data from MDR to memory
			else	//byte
				m_memory.byte_mem[MAR] = (unsigned char)MDR;	//write word data from MDR to memory
		}
	}
}