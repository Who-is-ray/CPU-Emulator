#include "CPU.h"
#include <iostream>
#include <bitset>

#define	READ	0
#define	WRITE	1
#define	BYTE	0
#define	WORD	1
#define	BYTE_SIZE	8
#define	LOWER_BYTE	0xff
#define	HIGHER_BYTE	0xff00
#define PROGRAM_COUNTER Register_file[7]
#define PSW	Register_file[6]
#define STACK_POINTER	Register_file[5]
#define LINK_REGISTER	Register_file[4]
#define	INVAILD_PC	0xffff
#define	PSW_CARRY	0b1
#define	PSW_ZERO	0b10
#define	PSW_NEGATIVE	0b100
#define	PSW_SLP		0b1000
#define	PSW_OVERFLOW	0b10000
#define	PSW_PRIORITY	0b11100000
#define PRPO	0b10000000000
#define DEC		0b1000000000
#define INC		0b100000000
#define R_C		0b10000000
#define	W_B		0b1000000
#define	DST		0b111
#define SRC		0b111000
#define DATA	0b11111111000
#define	BRANCH_OFFSET	0b1111111111
#define BRANCH_OFFSET_SIGN_BIT	0b1000000000
#define BIT_TWELVE		0b1000000000000
#define BL_SIGN_EXTENSION	0b1100000000000000
#define	RELATIVE_OFFSET	0b1111110000000
#define RELATIVE_SIGN_EXTENSION	0b1111111110000000

int opcode;	//opcode for decoding and executing

/*
	constructor
	initialize CPU clock
*/
CPU::CPU(unsigned char* memory):m_memory(memory)
{
	CPU_clock = 0;
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

//CPU fetch function, emulate fetch routine
void CPU::fetch()
{
	if (PROGRAM_COUNTER == INVAILD_PC)	//if program counter is invaild, return from interrupt
	{

	}
	else	//if program counter is vaild
	{
		MAR = PROGRAM_COUNTER;	//load program counter to MAR
		bus(READ);	//read through bus, load insturction to MDR
		IR = MDR;	//load instruction from MDR to IR
		PROGRAM_COUNTER += 2;	//update program counter
	}
}

//CPU decode function, emulate decode routine
void CPU::decode()
{
	unsigned char three_bits = IR >> 13;	//bit 15 to 13 of instruction ??magic number?
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
			opcode = IR >> 10;	//Opcode is bit 15 to 11	??magic number?
			break;
		}
		case 3:	//instruction is 011x xxxx xxxx xxxx
		{
			opcode = IR >> 8;	//Opcode is bit 15 to 8	??magic number?
			break;
		}
		case 4:	//instruction is 100x xxxx xxxx xxxx
		case 5:	//instruction is 101x xxxx xxxx xxxx
		{
			opcode = IR >> 11;	//Opcode is bit 15 to 11	??magic number?
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
		LINK_REGISTER = PROGRAM_COUNTER;	//load program counter to link register
		short sign = (IR & BIT_TWELVE)>>12;	//get sign bit and shift to bit 0, so sign = 0 or 1 if sign bit is 0 or 1	??magic number?
		short offset = IR << 1;	//shift offset to right 1 bit
		offset += BL_SIGN_EXTENSION * sign;	//offset sign extend
		PROGRAM_COUNTER += offset;	//update program counter
		break;
	}
	case 6:	//Opcode = 110 (LDR)
	{
		unsigned short source = Register_file[get_SRC()];	//get source memory address
		MAR = source + get_relative_offset();	//store source address + offset to MAR
		if ((IR & W_B) > 0)	//if processing byte size data
		{
			bus(READ, BYTE);	//access memory
			write_byte_to_dst(Register_file[get_DST()], static_cast<unsigned char>(MDR));	//store data from MDR to destination register
		}
		else	//if processing word size data
		{
			bus(READ);	//access memory
			Register_file[get_DST()] = MDR;	//store data from MDR to destination register
		}
		break;
	}
	case 7:	//Opcode = 111 (STR)
	{
		unsigned short dst_address = Register_file[get_DST()];	//get destination memory address
		MAR = dst_address + get_relative_offset();	//store source address + offset to MAR
		MDR = Register_file[get_SRC()];	//store register value to MDR
		if ((IR & W_B) > 0)	//if processing byte size data
			bus(WRITE, BYTE);	//access memory
		else	//if processing word size data
			bus(WRITE);	//access memory
		break;
	}
	case 8:	//Opcode = 001000 (BEQ/BZ)
	{
		if ((PSW & PSW_ZERO) > 0)	//if PSW.Z is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 9:	//Opcode = 001001 (BNE/BNZ)
	{
		if ((PSW & PSW_ZERO) == 0)	//if PSW.Z is not set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 10:	//Opcode = 001010 (BC/BHS)
	{
		if ((PSW & PSW_CARRY) > 0)	//if PSW.C is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 11:	//Opcode = 001011 (BNC/BLO)
	{
		if ((PSW & PSW_CARRY) == 0)	//if PSW.C is not set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 12:	//Opcode = 001100 (BN)
	{
		if ((PSW & PSW_NEGATIVE) > 0)	//if PSW.N is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 13:	//Opcode = 001101 (BGE)
	{
		if ((PSW & PSW_NEGATIVE) == (PSW & PSW_OVERFLOW))	//if (PSW.N XOR PSW.V) = 0 
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 14:	//Opcode = 001110 (BLT)
	{
		if ((PSW & PSW_NEGATIVE) != (PSW & PSW_OVERFLOW))	//if (PSW.N XOR PSW.V) != 0 
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 15:	//Opcode = 001111 (BAL)
	{
		PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case 16:	//Opcode = 10000 (LD)
	{
		if ((IR&DEC) > 0)	//if DEC set	??replace by case?
		{
			if ((IR&PRPO) > 0)	//pre-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[get_SRC()] -= 1;	//pre decrement action
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST], MDR);	//load data from MDR to DST register
				}
				else	//if processing word size data
				{
					Register_file[get_SRC()] -= 2;	//pre decrement action
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ);	//access memory
					Register_file[DST] = MDR;	//load data from MDR to DST register
				}
			}
			else	//post-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST], MDR);	//load data from MDR to DST register
					Register_file[get_SRC()] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ);	//access memory
					Register_file[DST] = MDR;	//load data from MDR to DST register
					Register_file[get_SRC()] -= 2;	//pre decrement action
				}
			}
		}
		else if ((IR&INC) > 0)	//if INC set
		{
			if ((IR&PRPO) > 0)	//pre-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[get_SRC()] += 1;	//pre increment action
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST], MDR);	//load data from MDR to DST register
				}
				else	//if processing word size data
				{
					Register_file[get_SRC()] += 2;	//pre increment action
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ);	//access memory
					Register_file[DST] = MDR;	//load data from MDR to DST register
				}
			}
			else	//post-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST], MDR);	//load data from MDR to DST register
					Register_file[get_SRC()] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
					bus(READ);	//access memory
					Register_file[DST] = MDR;	//load data from MDR to DST register
					Register_file[get_SRC()] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if ((IR&W_B) > 0)	//if processing byte size data
			{
				MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
				bus(READ, BYTE);	//access memory
				write_byte_to_dst(Register_file[DST], MDR);	//load data from MDR to DST register
			}
			else	//if processing word size data
			{
				MAR = Register_file[get_SRC()];	//load memory address stored in register to MAR
				bus(READ);	//access memory
				Register_file[DST] = MDR;	//load data from MDR to DST register
			}
		}
		break;
	}
	case 17:	//Opcode = 10000 (ST)
	{
		if ((IR&DEC) > 0)	//if DEC set	??replace by case?
		{
			if ((IR&PRPO) > 0)	//pre-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[get_DST()] -= 1;	//pre decrement action
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE, BYTE);	//access memory
				}
				else	//if processing word size data
				{
					Register_file[get_DST()] -= 2;	//pre decrement action
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE);	//access memory
				}
			}
			else	//post-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE, BYTE);	//access memory
					Register_file[get_DST()] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE);	//access memory
					Register_file[get_DST()] -= 2;	//pre decrement action
				}
			}
		}
		else if ((IR&INC) > 0)	//if INC set
		{
			if ((IR&PRPO) > 0)	//pre-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[get_DST()] += 1;	//pre increment action
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE, BYTE);	//access memory
				}
				else	//if processing word size data
				{
					Register_file[get_DST()] += 2;	//pre increment action
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE);	//access memory
				}
			}
			else	//post-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE, BYTE);	//access memory
					Register_file[get_DST()] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC];	//store data from DST register to MDR
					bus(WRITE);	//access memory
					Register_file[get_DST()] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if ((IR&W_B) > 0)	//if processing byte size data
			{
				MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
				MDR = Register_file[SRC];	//store data from DST register to MDR
				bus(WRITE, BYTE);	//access memory
			}
			else	//if processing word size data
			{
				MAR = Register_file[get_DST()];	//load memory address stored in register to MAR
				MDR = Register_file[SRC];	//store data from DST register to MDR
				bus(WRITE);	//access memory
			}
		}
		break;
	}
	case 18:	//Opcode = 10010 (MOVL)
	{
		write_byte_to_dst(Register_file[get_DST()], get_data());	//load data to destination register low byte
		break;
	}
	case 19:	//Opcode = 10011 (MOVLZ)
	{
		Register_file[get_DST()] &= 0;	//clear all bit of destination register
		Register_file[get_DST()] = get_data();	//load data to destination register low byte
		break;
	}
	case 20:	//Opcode = 10100 (MOVH)
	{
		unsigned short data = get_data() >> BYTE_SIZE;	//get data from insturction and shift to high byte
		Register_file[get_DST()] &= LOWER_BYTE;	//clear high byte of register
		Register_file[get_DST()] += data;	//load data to high byte of destination register
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
	case 117:	//Opcode = 0111 0101 (SWPB)	??need to update psw?
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
		unsigned short tmp_reg = Register_file[get_DST()];	//save DST register value to a temporary variable
		Register_file[get_DST()] = Register_file[get_SRC()];	//copy source value to destination
		Register_file[get_SRC()] = tmp_reg;	//copy temporary value to source
		break;
	}
	default:	//Error
	{
		std::cout << "\nOpcode has mistake\n";
		break;
	}
	}
}

//bus function	!!write untested
void CPU::bus(bool read_write, bool byte_word)
{	//!!if address < 16
	unsigned char high_byte, low_byte;
	if (byte_word)	//if processing word size data
	{
		if (read_write)	//if write
		{
			low_byte = static_cast<unsigned char>(MDR);	//get low byte value
			high_byte = static_cast<unsigned char>(MDR >> BYTE_SIZE);
			m_memory[MAR] = low_byte;
			m_memory[MAR + 1] = high_byte;
		}
		else	//if read
		{
			low_byte = m_memory[MAR];
			high_byte = m_memory[MAR + 1];
			MDR = high_byte;	//load high byte to MDR
			MDR = MDR << BYTE_SIZE;	//shift data to high byte of MDR
			MDR += low_byte;	//load low byte to MDR
		}
	}
	else	//if processing byte size data
	{
		if (read_write)	//if write
			m_memory[MAR] = static_cast<unsigned char>(MDR);	//load data from MDR to memory
		else	//if read
			write_byte_to_dst(MDR, m_memory[MAR]);	//load data byte to MDR lower byte
	}
	CPU_clock += 1;	//update CPU clock 1 cycle for accessing memory
}

//write a byte size data to destination lower byte
void CPU::write_byte_to_dst(unsigned short& dst, unsigned char data)
{
	dst &= HIGHER_BYTE;	//clear the lower byte
	dst += data;	//load data to dst
}

//return DST info from instruction
unsigned char CPU::get_DST()
{
	return static_cast<unsigned char>(IR & DST);	//return DST info from instruction
}

//return SRC info from instruction
unsigned char CPU::get_SRC()
{
	return static_cast<unsigned char>((IR&SRC) >> 3);	//return SRC info from instruction
}

//return relative offset value from instruction (LDR STR)
short CPU::get_relative_offset()
{
	bool sign = (IR & BIT_TWELVE) >> 12;	//get sign bit and shift to bit 0, so sign = 0 or 1 if sign bit is 0 or 1	??magic number?
	short offset = (IR & RELATIVE_OFFSET) >> 6;	//shift offset to right 1 bit
	offset += RELATIVE_SIGN_EXTENSION * sign;	//offset sign extend
	return offset;
}

//return data value from instruction	(MOVL, MOVLZ, MOVH)
unsigned char CPU::get_data()
{
	return static_cast<unsigned char>((IR&DATA) >> 3);	//return data value from instruction	(MOVL, MOVLZ, MOVH)
}

//return offset value from instruction	(BEQ/BZ to BAL)
short CPU::get_offset()
{
	bool sign = (IR & BRANCH_OFFSET_SIGN_BIT) >> 10;	//get sign bit and shift to bit 0, so sign = 0 or 1 if sign bit is 0 or 1	??magic number?
	short offset = (IR & BRANCH_OFFSET) << 1;	//shift offset to right 1 bit
	offset += BL_SIGN_EXTENSION * sign;	//offset sign extend
	return offset;
}