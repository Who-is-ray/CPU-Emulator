#include "CPU.h"

#define SIZE_OF_REGISTER	8
#define	READ	0
#define	WRITE	1
#define	BYTE	0
#define	WORD	1
#define	BYTE_SIZE	8
#define PROGRAM_COUNTER Register_file[7]
/*
	constructor
	initialize CPU clock
*/
CPU::CPU(unsigned char* memory):m_memory(memory)
{
	CPU_clock = 0;
	Register_file = new unsigned short[SIZE_OF_REGISTER] {NULL};	//initialize register file
}

//destructor
CPU::~CPU()
{
	delete Register_file;
}

//get specific register value
unsigned short CPU::get_register_val(int loc)
{
	return Register_file[loc];
}

//set a specific register value
void CPU::set_register_val(int address, unsigned short val)
{
	Register_file[address] = val;
}

//bus function
void CPU::bus(bool read_write, bool byte_word = 1)
{
	unsigned char high_byte, low_byte;
	if (byte_word)	//if word
	{
		if (read_write)	//if write
		{
			low_byte = static_cast<unsigned char>(MDR);
			high_byte = static_cast<unsigned char>(MDR >> BYTE_SIZE);
			m_memory[MAR] = low_byte;
			m_memory[MAR + 1] = high_byte;
		}
		else	//if read
		{
			MDR = m_memory[MAR];
		}
	}
	else	//if byte
	{
		if (read_write)	//if write
		{
			low_byte = static_cast<unsigned char>(MDR);
			m_memory[MAR] = low_byte;
		}
		else	//if read
		{
			MDR & 0xff00;	//clear the lower byte
			MDR += m_memory[MAR];	//load data byte to MDR lower byte
		}
	}
}

//CPU fetch function, emulate fetch routine
void CPU::fetch()
{
	MAR = PROGRAM_COUNTER;	//load program counter to MAR
	bus(READ);	//read through bus, load insturction to MDR
	IR = MDR;	//load instruction from MDR to IR
	PROGRAM_COUNTER += 2;	//update program counter
}