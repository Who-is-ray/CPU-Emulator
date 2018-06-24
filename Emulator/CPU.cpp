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
			MDR = high_byte;	//load high byte to MDR
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
	switch (opcode)
	{
	case 0:	//Opcode = 000 (BL)
	{

		break;
	}
	case 6:	//Opcode = 110 (LDR)
	{

		break;
	}
	case 7:	//Opcode = 111 (STR)
	{

		break;
	}
	case 8:	//Opcode = 001000 (BEQ/BZ)
	{

		break;
	}
	case 9:	//Opcode = 001001 (BNE/BNZ)
	{

		break;
	}
	case 10:	//Opcode = 001010 (BC/BHS)
	{

		break;
	}
	case 11:	//Opcode = 001011 (BNC/BLO)
	{

		break;
	}
	case 12:	//Opcode = 001100 (BN)
	{

		break;
	}
	case 13:	//Opcode = 001101 (BGE)
	{

		break;
	}
	case 14:	//Opcode = 001110 (BLT)
	{

		break;
	}
	case 15:	//Opcode = 001111 (BAL)
	{

		break;
	}
	case 16:	//Opcode = 10000 (LD)
	{

		break;
	}
	case 17:	//Opcode = 10000 (ST)
	{

		break;
	}
	case 18:	//Opcode = 10010 (MOVL)
	{

		break;
	}
	case 19:	//Opcode = 10011 (MOVLZ)
	{

		break;
	}
	case 20:	//Opcode = 10100 (MOVH)
	{

		break;
	}
	case 96:	//Opcode = 0110 0000 (ADD)
	{

		break;
	}
	case 98:	//Opcode = 0110 0010 (ADDC)
	{

		break;
	}
	case 100:	//Opcode = 0110 0100 (SUB)
	{

		break;
	}
	case 102:	//Opcode = 0110 0110 (SUBC)
	{

		break;
	}
	case 104:	//Opcode = 0110 1000 (DADD)
	{

		break;
	}
	case 106:	//Opcode = 0110 1010 (CMP)
	{

		break;
	}
	case 108:	//Opcode = 0110 1100 (XOR)
	{

		break;
	}
	case 110:	//Opcode = 0110 1110 (AND)
	{

		break;
	}
	case 112:	//Opcode = 0111 0000 (BIT)
	{

		break;
	}
	case 113:	//Opcode = 0111 0001 (SRA)
	{

		break;
	}
	case 114:	//Opcode = 0111 0010 (BIC)
	{

		break;
	}
	case 115:	//Opcode = 0111 0011 (RRC)
	{

		break;
	}
	case 116:	//Opcode = 0111 0100 (BIS)
	{

		break;
	}
	case 117:	//Opcode = 0111 0101 (SWPB)
	{

		break;
	}
	case 118:	//Opcode = 0111 0110 (MOV)
	{

		break;
	}
	case 119:	//Opcode = 0111 0111 (SXT)
	{

		break;
	}
	case 120:	//Opcode = 0111 1000 (SWAP)
	{

		break;
	}
	default:
		break;
	}
}