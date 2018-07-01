#include "Memory.h"	//header that define memory class

/*
	bus function
	MAR - memory address
	MDR - reference to data to read/write
	rw	- read|write
	bw	- byte|word
*/
void Memory::bus(unsigned short MAR, unsigned short& MDR, ACTION rw, SIZE bw)
{
	if (rw == READ)
		MDR = (bw == WORD) ? m_memory.word_mem[MAR >> 1] : m_memory.byte_mem[MAR];	//read word/byte data to MDR
	else	//write
	{
		if (bw == WORD)
			m_memory.word_mem[MAR >> 1] = MDR;	//write word data from MDR to memory
		else	//byte
			m_memory.byte_mem[MAR] = (unsigned char)MDR;	//write word data from MDR to memory
	}
	if (MAR < 16)	//if accessing device SCR
		update_CSR(rw, bw, MAR);
	m_clock++;	//increment of clock when accessing data
}

//update CSR after process with CSR.data
void Memory::update_CSR(ACTION rw, SIZE bw, unsigned short address)
{
	if (bw == WORD)	//if processing word size data, process with CSR.data
	{
		if ((m_memory.byte_mem[address] & CSR_IO) > 0 && rw == READ)	//if reading from input device's control/status register
			m_memory.byte_mem[address] &= ~(CSR_DBA | CSR_OF);	//clear OF and DBA
		else if ((m_memory.byte_mem[address] & CSR_IO) == 0 && rw == WRITE)	//if writing to output device's control/status register
			m_memory.byte_mem[address] |= (m_memory.byte_mem[address] & CSR_DBA) > 0 ? CSR_OF : CSR_DBA;	//set DBA, set OF if DBA was set
	}
}