#include "CPU.h"
#include "Memory.h"
#include <iostream>
#include <bitset>

#define	BYTE_SIZE	8
#define	LOWER_BYTE	0xff
#define	HIGHER_BYTE	0xff00
#define PROGRAM_COUNTER Register_file[7]
#define PSW				Register_file[6]
#define STACK_POINTER	Register_file[5]
#define LINK_REGISTER	Register_file[4]
#define	INVAILD_PC	0xffff
#define	PSW_CARRY		1
#define	PSW_ZERO		1<<1
#define	PSW_NEGATIVE	1<<2
#define	PSW_SLP			1<<3
#define	PSW_OVERFLOW	1<<4
#define	PSW_PRIORITY	0b11100000
#define PRPO	1<<10
#define DEC		1<<9
#define INC		1<<8
#define R_C		1<<7
#define	W_B		1<<6
#define	DST		0b111
#define SRC		0b111000
#define DATA	0b11111111000
#define	BRANCH_OFFSET	0b1111111111
#define BRANCH_OFFSET_SIGN_BIT	1<<9
#define BRANCH_OFFSET_SIGN_EXTENSION 0b1111100000000000
#define BIT_ONE 1
#define BIT_TWELVE		1<<11
#define BIT_FOURTEEN	1<<13
#define BIT_FIFTEEN		1<<14
#define BL_SIGN_EXTENSION	0b1100000000000000
#define	RELATIVE_OFFSET	0b1111110000000
#define RELATIVE_SIGN_EXTENSION	0b1111111110000000
#define BYTE_SIGN_BIT	1<<7
#define WORD_SIGN_BIT	1<<15
#define BYTE_CARRY_BIT	1<<8
#define WORD_CARRY_BIT	1<<16

int opcode;	//opcode for decoding and executing

/*
	constructor
	linked memory reference to CPU memory reference
	initialize CPU clock
	memory - memory of emulator
*/
CPU::CPU(Memory& memory) :m_mem(memory), CPU_clock(0)
{}

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
	printf("PC = %4lx\n", PROGRAM_COUNTER);
	if (PROGRAM_COUNTER == INVAILD_PC)	//if program counter is invaild, return from interrupt
	{

	}
	else	//if program counter is vaild
	{
		MAR = PROGRAM_COUNTER;	//load program counter to MAR
		m_mem.bus(MAR, MDR, READ);	//read through bus, load insturction to MDR
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
		unsigned short source = Register_file[SRC_EA()];	//get source memory address
		MAR = source + get_relative_offset();	//store source address + offset to MAR
		if ((IR & W_B) > 0)	//if processing byte size data
		{
			m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//store data from MDR to destination register
		}
		else	//if processing word size data
		{
			m_mem.bus(MAR, MDR, READ);	//access memory
			Register_file[DST_EA()] = MDR;	//store data from MDR to destination register
		}
		break;
	}
	case 7:	//Opcode = 111 (STR)
	{
		unsigned short dst_address = Register_file[DST_EA()];	//get destination memory address
		MAR = dst_address + get_relative_offset();	//store source address + offset to MAR
		MDR = Register_file[SRC_EA()];	//store register value to MDR
		(IR & W_B) > 0 ? m_mem.bus(MAR, MDR, WRITE, BYTE) : m_mem.bus(MAR, MDR, WRITE);	//access memory byte|word size data
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
					Register_file[SRC_EA()] -= 1;	//pre decrement action
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//load data from MDR to DST register
				}
				else	//if processing word size data
				{
					Register_file[SRC_EA()] -= 2;	//pre decrement action
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ);	//access memory
					Register_file[DST_EA()] = MDR;	//load data from MDR to DST register
				}
			}
			else	//post-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//load data from MDR to DST register
					Register_file[SRC_EA()] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ);	//access memory
					Register_file[DST_EA()] = MDR;	//load data from MDR to DST register
					Register_file[SRC_EA()] -= 2;	//pre decrement action
				}
			}
		}
		else if ((IR&INC) > 0)	//if INC set
		{
			if ((IR&PRPO) > 0)	//pre-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[SRC_EA()] += 1;	//pre increment action
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//load data from MDR to DST register
				}
				else	//if processing word size data
				{
					Register_file[SRC_EA()] += 2;	//pre increment action
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ);	//access memory
					Register_file[DST_EA()] = MDR;	//load data from MDR to DST register
				}
			}
			else	//post-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
					write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//load data from MDR to DST register
					Register_file[SRC_EA()] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
					m_mem.bus(MAR, MDR, READ);	//access memory
					Register_file[DST_EA()] = MDR;	//load data from MDR to DST register
					Register_file[SRC_EA()] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if ((IR&W_B) > 0)	//if processing byte size data
			{
				MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
				m_mem.bus(MAR, MDR, READ, BYTE);	//access memory
				write_byte_to_dst(Register_file[DST_EA()], (unsigned char)MDR);	//load data from MDR to DST register
			}
			else	//if processing word size data
			{
				MAR = Register_file[SRC_EA()];	//load memory address stored in register to MAR
				m_mem.bus(MAR, MDR, READ);	//access memory
				Register_file[DST_EA()] = MDR;	//load data from MDR to DST register
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
					Register_file[DST_EA()] -= 1;	//pre decrement action
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE, BYTE);	//access memory
				}
				else	//if processing word size data
				{
					Register_file[DST_EA()] -= 2;	//pre decrement action
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE);	//access memory
				}
			}
			else	//post-decrement action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE, BYTE);	//access memory
					Register_file[DST_EA()] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE);	//access memory
					Register_file[DST_EA()] -= 2;	//pre decrement action
				}
			}
		}
		else if ((IR&INC) > 0)	//if INC set
		{
			if ((IR&PRPO) > 0)	//pre-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					Register_file[DST_EA()] += 1;	//pre increment action
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE, BYTE);	//access memory
				}
				else	//if processing word size data
				{
					Register_file[DST_EA()] += 2;	//pre increment action
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE);	//access memory
				}
			}
			else	//post-increment action
			{
				if ((IR&W_B) > 0)	//if processing byte size data
				{
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE, BYTE);	//access memory
					Register_file[DST_EA()] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
					MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
					m_mem.bus(MAR, MDR, WRITE);	//access memory
					Register_file[DST_EA()] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if ((IR&W_B) > 0)	//if processing byte size data
			{
				MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
				MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
				m_mem.bus(MAR, MDR, WRITE, BYTE);	//access memory
			}
			else	//if processing word size data
			{
				MAR = Register_file[DST_EA()];	//load memory address stored in register to MAR
				MDR = Register_file[SRC_EA()];	//store data from DST register to MDR
				m_mem.bus(MAR, MDR, WRITE);	//access memory
			}
		}
		break;
	}
	case 18:	//Opcode = 10010 (MOVL)
	{
		write_byte_to_dst(Register_file[DST_EA()], get_data());	//load data to destination register low byte
		break;
	}
	case 19:	//Opcode = 10011 (MOVLZ)
	{
		Register_file[DST_EA()] &= 0;	//clear all bit of destination register
		Register_file[DST_EA()] = get_data();	//load data to destination register low byte
		break;
	}
	case 20:	//Opcode = 10100 (MOVH)
	{
		Register_file[DST_EA()] &= LOWER_BYTE;	//clear high byte of register
		Register_file[DST_EA()] += (get_data() << BYTE_SIZE);	//load data to high byte of destination register and shift to high byte
		break;
	}
	case 96:	//Opcode = 0110 0000 (ADD)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2;	//add data1 +data2 to result
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT,BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2;	//add data1 +data2 to result
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 98:	//Opcode = 0110 0010 (ADDC)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 100:	//Opcode = 0110 0100 (SUB)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = ~((unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = ~((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 102:	//Opcode = 0110 0110 (SUBC)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = ~((unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = ~((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 104:	//Opcode = 0110 1000 (DADD)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char) Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + ((PSW&PSW_CARRY) > 1 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 106:	//Opcode = 0110 1010 (CMP)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char) Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = ~((unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = ~((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 108:	//Opcode = 0110 1100 (XOR)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2) + (data2 & ~data1);	//do XOR operation
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2) + (data2 & ~data1);	//do XOR operation
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 110:	//Opcode = 0110 1110 (AND)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do AND operation
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do AND operation
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 112:	//Opcode = 0111 0000 (BIT)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do BIT operation
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do BIT operation
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 113:	//Opcode = 0111 0001 (SRA)
	{
		//PSW |= (Register_file[DST_EA()] & BIT_ONE) > 0 ? PSW_CARRY : 0;	//set lsb to carry
		//Register_file[DST_EA()] = Register_file[DST_EA()] >> 1;	//shift right one bit
		//Register_file[DST_EA()] |= (Register_file[DST_EA()] & BIT_FOURTEEN) > 1 ? BIT_FIFTEEN : 0;	//set msb if have sign
		break;
	}
	case 114:	//Opcode = 0111 0010 (BIC)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2);	//do BIT operation
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2);	//do BIT operation
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 115:	//Opcode = 0111 0011 (RRC)
	{

		break;
	}
	case 116:	//Opcode = 0111 0100 (BIS)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)((IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 | data2);	//do AND operation
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST_EA()];	//get data from reigister[DST]
			unsigned short data2 = (IR&R_C) > 1 ? const_table[SRC_EA()] : Register_file[SRC_EA()];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 | data2);	//do AND operation
			Register_file[DST_EA()] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case 117:	//Opcode = 0111 0101 (SWPB)	??need to update psw?
	{

		break;
	}
	case 118:	//Opcode = 0111 0110 (MOV)
	{
		if ((IR&W_B) > 0)	//if processing byte size data
			write_byte_to_dst(Register_file[DST_EA()], (unsigned char)Register_file[SRC_EA()]);	//copy data from src to dst
		else
			Register_file[DST_EA()] = Register_file[SRC_EA()];	//copy data from src to dst
		break;
	}
	case 119:	//Opcode = 0111 0111 (SXT)
	{

		break;
	}
	case 120:	//Opcode = 0111 1000 (SWAP)
	{
		unsigned short tmp_reg = Register_file[DST_EA()];	//save DST register value to a temporary variable
		Register_file[DST_EA()] = Register_file[SRC_EA()];	//copy source value to destination
		Register_file[SRC_EA()] = tmp_reg;	//copy temporary value to source
		break;
	}
	default:	//Error
	{
		std::cout << "\nOpcode has mistake\n";
		break;
	}
	}
}

/*
	write a byte size data to destination lower byte
	dst - destination word data
	data - byte data to load
*/
void CPU::write_byte_to_dst(unsigned short& dst, unsigned char data)
{
	dst &= HIGHER_BYTE;	//clear the lower byte
	dst += data;	//load data to dst
}

//return DST info from instruction
unsigned char CPU::DST_EA()
{
	return static_cast<unsigned char>(IR & DST);	//return DST info from instruction
}

//return SRC info from instruction
unsigned char CPU::SRC_EA()
{
	return static_cast<unsigned char>((IR&SRC) >> 3);	//return SRC info from instruction
}

//return relative offset value from instruction (LDR STR)
short CPU::get_relative_offset()
{
	short offset = (IR & RELATIVE_OFFSET) >> 6;	//get offset
	offset += (IR & BIT_TWELVE) > 0 ? RELATIVE_SIGN_EXTENSION : 0;	//offset sign extend
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
	short offset = (IR & BRANCH_OFFSET) << 1;	//shift offset to right 1 bit
	offset += (IR & BRANCH_OFFSET_SIGN_BIT) > 0 ? BRANCH_OFFSET_SIGN_EXTENSION : 0;	//offset sign extend
	return offset;
}

/*
	updat psw register
	result - result to check status flags

*/
void CPU::ModifyStatusFlags(unsigned int result, unsigned int data1, unsigned int data2, unsigned int carry_bit, unsigned int sign_bit)
{
	//indicate setting carry flag
	if ((result&carry_bit) > 0)	//if has carry
	{
		PSW |= PSW_CARRY;	//set carry
		result -= carry_bit;	//unset carry bit from zero checking
	}
	else	//if no carry
		PSW &= ~PSW_CARRY;	//clear carry

	//indicate setting overflow flag
	if ((data1&sign_bit) == (data2&sign_bit))	//if source and destination have same sign
	{
		if ((data1&sign_bit) != (result&sign_bit))	//if result and source have different sign
			PSW |= PSW_OVERFLOW;	//set overflow flag
		else //clear overflow
			PSW &= ~PSW_OVERFLOW;	//clear overflow flag
	}
	else //clear overflow
		PSW &= ~PSW_OVERFLOW;	//clear overflow flag

	//indicate setting negative flag
	if ((result&sign_bit) > 0)	//if result is negative
		PSW |= PSW_NEGATIVE;	//set carry
	else //clear negative
		PSW &= ~PSW_NEGATIVE;	//clear negative flag

	//indicate setting zero flag
	if (result == 0)
		PSW |= PSW_ZERO;	//set zero
	else //clear zero
		PSW &= ~PSW_ZERO;	//clear zero flag
}