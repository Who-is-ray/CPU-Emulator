#include "CPU.h"
#include <iostream>

#define SIZE_OF_REGISTER	8
#define	READ	0
#define	WRITE	1
#define	BYTE	0
#define	WORD	1
#define	BYTE_SIZE	8
#define PROGRAM_COUNTER Register_file[7]

int opcode;	//opcode for decoding and executing

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

//bus function	!!Untested
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
			low_byte = m_memory[MAR];
			high_byte = m_memory[MAR + 1];
			MDR += high_byte;	//load high byte to MDR
			MDR = MDR << 8;	//shift data to high byte of MDR
			MDR += low_byte;	//load low byte to MDR
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
	CPU_clock += 1;	//update CPU clock 1 cycle for fetching
}

//CPU decode function, emulate decode routine
void CPU::decode()
{
	unsigned char three_bits = IR >> 13;	//bit 15 to 13 of instruction
	switch (three_bits)
	{
		case 0: //instruction is 000x xxxx xxxx xxxx
		case 6:	//instruction is 110x xxxx xxxx xxxx
		case 7:	//instruction is 111x xxxx xxxx xxxx
		{
			opcode = three_bits;	//Opcode is bit 15 to 13
			break;
		}
		case 1:	//instruction is 001x xxxx xxxx xxxx
		{
			opcode = IR >> 10;	//Opcode is bit 15 to 11
			break;
		}
		case 3:	//instruction is 011x xxxx xxxx xxxx
		{
			opcode = IR >> 8;	//Opcode is bit 15 to 8
			break;
		}
		case 4:	//instruction is 100x xxxx xxxx xxxx
		case 5:	//instruction is 101x xxxx xxxx xxxx
		{
			opcode = IR >> 11;	//Opcode is bit 15 to 11
			break;
		}
		default:	//Error
		{
			std::cout << "\nOpcode has mistake\n";
			break;
		}
	}
}

//CPU execute function, emulate execute routine
void CPU::execute()
{
	
}