#include "CPU.h"	//CPU class header file
#include "Cache_Memory.h"	//Cache class header file
#include "Memory.h"	//Memory class header file
#include <iostream>	//library for concole output
#include <fstream>	//library for accessing file

#define	BYTE_SIZE	8		//size of byte
#define	LOWER_BYTE	0xff	//low byte data position in a word
#define	HIGHER_BYTE	0xff00	//high byte data position in a word
#define PROGRAM_COUNTER Register_file[7]
#define PSW				Register_file[6]
#define STACK_POINTER	Register_file[5]
#define LINK_REGISTER	Register_file[4]
#define GROUP_CODE			(IR >> 13)	//code of group of instructions
#define GROUP_ONE_OPCODE	(IR >> 10)	//opcode of group one, bit 15 to 10
#define GROUP_TWO_OPCODE	(IR >> 8)		//opcode of group two, bit 15 to 8
#define GROUP_THREE_OPCODE	(IR >> 11)	//opcode of group three, bit 15 to 11
#define	INVAILD_PC		0xffff			//invaild program counter value return from interrupt
#define GET_CARRY(x)	(x&1)			//value of psw.carry
#define	GET_ZERO(x)		(x&(1<<1))	//value of psw.zero
#define	GET_NEGATIVE(x)	(x&(1<<2))	//value of psw.negative
#define	GET_SLP	(x)		(x&(1<<3))	//value of psw.sleep
#define	GET_OVERFLOW(x)	(x&(1<<4))	//value of psw.overflow
#define CARRY			1			//value of psw.carry
#define	ZERO			(1<<1)		//value of psw.zero
#define	NEGATIVE		(1<<2)		//value of psw.negative
#define	SLP				(1<<3)		//value of psw.sleep
#define	OVER_FLOW		(1<<4)		//value of psw.overflow
#define	GET_PRIORITY(x)	(unsigned char)(x&0b11100000)>>5	//value of psw.priority
#define GET_DATA(x)		(unsigned char)(x>>8)	//get psw.data
#define PRPO	(IR&(1<<10))	//PRPO bit of instruction bit 11
#define DEC		(IR&(1<<9))		//DEC bit of instruction bit 10
#define INC		(IR&(1<<8))		//INC bit of instruction bit 9
#define R_C		(IR&(1<<7))		//R_C bit of instruction bit 8
#define	W_B		(IR&(1<<6))		//W_B bit of instruction bit 7
#define	DST		(unsigned char)(IR&0b111)	//DST value from instruction bit 2 to 0
#define SRC		(unsigned char)((IR&0b111000)>>3)	//SRC value from instruction bit 5 to 3
#define DATA	(unsigned short)((IR&0b11111111000) >> 3)	//Data value from instruction bit 10 to 3
#define	BRANCH_OFFSET					(IR&0b1111111111)<<1//get value of branch offset
#define BRANCH_OFFSET_SIGN				(IR & 1<<9)			//sign bit of branch offset
#define BRANCH_OFFSET_SIGN_EXTENSION	0b1111100000000000	//sign extension of branch offset
#define BL_SIGN_EXTENSION				0b1100000000000000	//BL sign extension
#define	RELATIVE_OFFSET					(IR & 0b1111110000000) >> 7	//get the value of relative offset
#define RELATIVE_SIGN_EXTENSION			0b1111111110000000	//sign extension of relative sign extension
#define BYTE_SIGN_BIT	(1<<7)	//sign bit of byte data
#define WORD_SIGN_BIT	(1<<15)	//sign bit of word data
#define BYTE_CARRY_BIT	(1<<8)	//byte data carry bit
#define WORD_CARRY_BIT	(1<<16)	//word data carry bit
#define BYTE_CARRY_BIT_DEC	100	//decimal byte carry bit
#define WORD_CARRY_BIT_DEC	10000	//decimal word carry bit
#define BIT_ONE			1	
#define BIT_SIX			(1<<6)
#define BIT_SEVEN		(1<<7)
#define BIT_TWELVE		(1<<11)
#define BIT_FOURTEEN	(1<<14)
#define BIT_FIFTEEN		(1<<15)
#define GET_DEV_VECTOR_ADDR(X)	(0xFFC0+(X*4))	//get device vector address of a device
#define GET_1ST_FOUR_BITS(X)	(X&0xf)			//get the first four bits
#define GET_2ND_FOUR_BITS(X)	((X&0xf0)>>4)	//get the second four bitss
#define GET_3RD_FOUR_BITS(X)	((X&0xf00)>>8)	//get thrid four bits
#define GET_4TH_FOUR_BITS(X)	((X&0xf000)>>12)	//get forth four bits

//enum of opcode
enum Opcode
{
	BL,			LDR=6,		STR,		BEQ_BZ,		BNE_BNZ,
	BC_BHS,		BNC_BLO,	BN,			BGE,		BLT,
	BAL,		LD,			ST,			MOVL,		MOVLZ,
	MOVH,		ADD=96,		ADDC=98,	SUB=100,	SUBC=102,
	DADD=104,	CMP=106,	XOR=108,	AND=110,	BIT=112,
	SRA,		BIC,		RRC,		BIS,		SWPB,
	MOV,		SXT,		SWAP

};

int opcode;	//opcode for decoding and executing
std::ofstream fout;	//device output file

int hex_to_dec(unsigned short data);
unsigned short dec_to_hex(int data);

/*
	constructor
	linked memory reference to CPU memory reference
	memory - memory of emulator
*/
CPU::CPU(Memory& memory, Cache_Memory& cache, unsigned int& clock) :m_cache(cache), m_clock(clock), m_mem(memory)
{
	fout.open("device_output.txt");	//initialize device output file
}

//CPU fetch function, emulate fetch routine
void CPU::fetch()
{
	if (PROGRAM_COUNTER == INVAILD_PC)	//if program counter is invaild, return from interrupt
	{
		if (interrput_queue.size() > 0)	//if there is pending interrupt at the end of last interrupt, process the earilest interrupt
		{
			//get the base routine's priority, which is store in the stack frame
			m_cache.cache(STACK_POINTER + 2, MDR, READ, BYTE);
			unsigned short base_priority = MDR;

			std::deque<std::map<unsigned char /*priority*/, unsigned char /*device number*/>>::iterator it = interrput_queue.begin();	//get the earliest interrupt map in the queue
			unsigned char dev_priority = it->rbegin()->first;	//get interrupt's priority

			if (dev_priority > base_priority)	//if interrupt has higher priority than return routine
			{
				unsigned char dev_num = it->rbegin()->second;
				//the handler's PSW becomes the current PSW
				m_cache.cache(GET_DEV_VECTOR_ADDR(dev_num), MDR, READ);
				PSW = MDR;

				//PC points to ISR
				m_cache.cache(GET_DEV_VECTOR_ADDR(dev_num) + 2, MDR, READ);
				PROGRAM_COUNTER = MDR;

				it->erase(dev_priority);	//erase the pending interrupt from interrupt map of a time point
				if (it->size() == 0)	//if the interrupt map of a time point is empty, erase this map from interrupt queue
					interrput_queue.pop_front();
			}
			else
				return_from_interrupt();
		}
		else	//there is no more pending interrupt
			return_from_interrupt();
	}

	//if program counter is vaild
	MAR = PROGRAM_COUNTER;	//load program counter to MAR
	m_cache.cache(MAR, MDR, READ);	//read through bus, load insturction to MDR
	IR = MDR;	//load instruction from MDR to IR
	PROGRAM_COUNTER += 2;	//update program counter
}

/*
	CPU decode function, emulate decode routine
	get the opcode from of instruction
*/
void CPU::decode()
{
	unsigned char group_code = GROUP_CODE;	//check bit 15 to 13 of instruction to decide the size of Opcode
	switch (group_code)
	{
		case 0: //instruction is 000x xxxx xxxx xxxx
		case 6:	//instruction is 110x xxxx xxxx xxxx
		case 7:	//instruction is 111x xxxx xxxx xxxx
		{
			opcode = group_code;	//Opcode is bit 15 to 13
			break;
		}
		case 1:	//instruction is 001x xxxx xxxx xxxx
		{
			opcode = GROUP_ONE_OPCODE;	//Opcode is bit 15 to 11
			break;
		}
		case 3:	//instruction is 011x xxxx xxxx xxxx
		{
			opcode = GROUP_TWO_OPCODE;	//Opcode is bit 15 to 8
			break;
		}
		case 4:	//instruction is 100x xxxx xxxx xxxx
		case 5:	//instruction is 101x xxxx xxxx xxxx
		{
			opcode = GROUP_THREE_OPCODE;	//Opcode is bit 15 to 11
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
	m_clock++;	//increment of clock for all instruction
	switch (opcode)
	{
	case BL:	//Opcode = 000 (BL)
	{
		LINK_REGISTER = PROGRAM_COUNTER;	//load program counter to link register
		short offset = IR << 1;	//shift offset to right 1 bit
		offset += (IR & BIT_TWELVE) > 0 ? BL_SIGN_EXTENSION : 0;	//check sign of offset and do offset sign extend
		PROGRAM_COUNTER += offset;	//update program counter
		break;
	}
	case LDR:	//Opcode = 110 (LDR)
	{
		unsigned short source = Register_file[SRC];	//get source memory address
		MAR = source + get_relative_offset();	//store source address + offset to MAR
		if (W_B > 0)	//if processing byte size data
		{
			m_cache.cache(MAR, MDR, READ, BYTE);	//access memory
			write_byte_to_dst(Register_file[DST], (unsigned char)MDR);	//store data from MDR to destination register
		}
		else	//if processing word size data
		{
			m_cache.cache(MAR, MDR, READ);	//access memory
			Register_file[DST] = MDR;	//store data from MDR to destination register
		}
		break;
	}
	case STR:	//Opcode = 111 (STR)
	{
		unsigned short dst_address = Register_file[DST];	//get destination memory address
		MAR = dst_address + get_relative_offset();	//store source address + offset to MAR
		MDR = Register_file[SRC];	//store register value to MDR
		W_B > 0 ? m_cache.cache(MAR, MDR, WRITE, BYTE) : m_cache.cache(MAR, MDR, WRITE);	//access memory byte|word size data
		break;
	}
	case BEQ_BZ:	//Opcode = 001000 (BEQ/BZ)
	{
		if (GET_ZERO(PSW) > 0)	//if PSW.Z is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BNE_BNZ:	//Opcode = 001001 (BNE/BNZ)
	{
		if (GET_ZERO(PSW) == 0)	//if PSW.Z is not set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BC_BHS:	//Opcode = 001010 (BC/BHS)
	{
		if (GET_CARRY(PSW) > 0)	//if PSW.C is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BNC_BLO:	//Opcode = 001011 (BNC/BLO)
	{
		if (GET_CARRY(PSW) == 0)	//if PSW.C is not set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BN:	//Opcode = 001100 (BN)
	{
		if (GET_NEGATIVE(PSW) > 0)	//if PSW.N is set
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BGE:	//Opcode = 001101 (BGE)
	{
		if (GET_NEGATIVE(PSW) == GET_OVERFLOW(PSW))	//if (PSW.N XOR PSW.V) = 0 
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BLT:	//Opcode = 001110 (BLT)
	{
		if (GET_NEGATIVE(PSW) != GET_OVERFLOW(PSW))	//if (PSW.N XOR PSW.V) != 0 
			PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case BAL:	//Opcode = 001111 (BAL)
	{
		PROGRAM_COUNTER += get_offset();	//Update program counter with offset
		break;
	}
	case LD:	//Opcode = 10000 (LD)
	{
		if ((IR&DEC) > 0)	//if DEC set
		{
			if (PRPO > 0)	//pre-decrement action
			{
				if (W_B > 0)	//if processing byte size data
				{
					Register_file[SRC] -= 1;	//pre decrement action
					load_byte();
				}
				else	//if processing word size data
				{
					Register_file[SRC] -= 2;	//pre decrement action
					load_word();
				}
			}
			
			
			else	//post-decrement action
			{
				if (W_B > 0)	//if processing byte size data
				{
					load_byte();
					Register_file[SRC] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					load_word();
					Register_file[SRC] -= 2;	//pre decrement action
				}
			}
		}
		else if (INC > 0)	//if INC set
		{
			if (PRPO > 0)	//pre-increment action
			{
				if (W_B > 0)	//if processing byte size data
				{
					Register_file[SRC] += 1;	//pre increment action
					load_byte();
				}
				else	//if processing word size data
				{
					Register_file[SRC] += 2;	//pre increment action
					load_word();
				}
			}
			else	//post-increment action
			{
				if (W_B > 0)	//if processing byte size data
				{
					load_byte();
					Register_file[SRC] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					load_word();
					Register_file[SRC] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if (W_B > 0)	//if processing byte size data
				load_byte();
			else	//if processing word size data
				load_word();
		}
		break;
	}
	case ST:	//Opcode = 10000 (ST)
	{
		if ((IR&DEC) > 0)	//if DEC set
		{
			if (PRPO > 0)	//pre-decrement action
			{
				if (W_B > 0)	//if processing byte size data
				{
					Register_file[DST] -= 1;	//pre decrement action
					store_byte();
				}
				else	//if processing word size data
				{
					Register_file[DST] -= 2;	//pre decrement action
					store_word();
				}
			}
			else	//post-decrement action
			{
				if (W_B > 0)	//if processing byte size data
				{
					store_byte();
					Register_file[DST] -= 1;	//post decrement action
				}
				else	//if processing word size data
				{
					store_word();
					Register_file[DST] -= 2;	//pre decrement action
				}
			}
		}
		else if (INC > 0)	//if INC set
		{
			if (PRPO > 0)	//pre-increment action
			{
				if (W_B > 0)	//if processing byte size data
				{
					Register_file[DST] += 1;	//pre increment action
					store_byte();
				}
				else	//if processing word size data
				{
					Register_file[DST] += 2;	//pre increment action
					store_word();
				}
			}
			else	//post-increment action
			{
				if (W_B > 0)	//if processing byte size data
				{
					store_byte();
					Register_file[DST] += 1;	//post increment action
				}
				else	//if processing word size data
				{
					store_word();
					Register_file[DST] += 2;	//pre increment action
				}
			}
		}
		else	//if either DEC or INC set
		{
			if (W_B > 0)	//if processing byte size data
			{
				store_byte();
			}
			else	//if processing word size data
			{
				store_word();
			}
		}
		break;
	}
	case MOVL:	//Opcode = 10010 (MOVL)
	{
		write_byte_to_dst(Register_file[DST], (unsigned char)DATA);	//load data to destination register low byte
		break;
	}
	case MOVLZ:	//Opcode = 10011 (MOVLZ)
	{
		Register_file[DST] &= 0;	//clear all bit of destination register
		Register_file[DST] = DATA;	//load data to destination register low byte
		break;
	}
	case MOVH:	//Opcode = 10100 (MOVH)
	{
		Register_file[DST] &= LOWER_BYTE;	//clear high byte of register
		Register_file[DST] |= (DATA << BYTE_SIZE);	//load data to high byte of destination register and shift to high byte
		break;
	}
	case ADD:	//Opcode = 0110 0000 (ADD)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2;	//add data1 +data2 to result
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT,BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2;	//add data1 +data2 to result
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case ADDC:	//Opcode = 0110 0010 (ADDC)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case SUB:	//Opcode = 0110 0100 (SUB)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			char data2 = ~((unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			short data2 = ~(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case SUBC:	//Opcode = 0110 0110 (SUBC)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			char data2 = ~((unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			short data2 = ~(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case DADD:	//Opcode = 0110 1000 (DADD)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned int data1 = hex_to_dec(Register_file[DST]);	//get data from reigister[DST]
			unsigned int data2 = hex_to_dec(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			write_byte_to_dst(Register_file[DST], (unsigned char)dec_to_hex(result));	//load result to register
			ModifyStatusFlags_dec(result, data1, data2, BYTE_CARRY_BIT_DEC);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned int data1 = hex_to_dec(Register_file[DST]);	//get data from reigister[DST]
			unsigned int data2 = hex_to_dec(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + (GET_CARRY(PSW) > 0 ? 1 : 0);	//add data1 +data2 to result, and add carry to result
			Register_file[DST] = dec_to_hex(result);	//load result to register
			ModifyStatusFlags_dec(result, data1, data2, WORD_CARRY_BIT_DEC);	//update psw register
		}
		break;
	}
	case CMP:	//Opcode = 0110 1010 (CMP)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char) Register_file[DST];	//get data from reigister[DST]
			char data2 = ~((unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]));	//get negative data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			short data2 = ~(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = data1 + data2 + 1;	//add data1 + data2 + 1 to result
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case XOR:	//Opcode = 0110 1100 (XOR)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2) | (data2 & ~data1);	//do XOR operation
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2) | (data2 & ~data1);	//do XOR operation
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case AND:	//Opcode = 0110 1110 (AND)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do AND operation
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do AND operation
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case BIT:	//Opcode = 0111 0000 (BIT)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do BIT operation
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & data2);	//do BIT operation
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case SRA:	//Opcode = 0111 0001 (SRA)
	{
		PSW |= (Register_file[DST] & BIT_ONE) > 0 ? 1 : 0;	//set lsb to carry
		if (W_B > 0)	//if processing byte size data
		{
			unsigned short LSByte = Register_file[DST] & LOWER_BYTE;	//get lowbyte of destination register
			Register_file[DST] &= HIGHER_BYTE;	//clear loybyte of destination register
			LSByte = LSByte >> 1;	//shift right one bit
			LSByte |= (Register_file[DST] & BIT_SIX) > 0 ? BIT_SEVEN : 0;	//set msb if have sign
			Register_file[DST] |= LSByte;	//store back to register
		}
		else	//if processing word size data
		{
			Register_file[DST] = Register_file[DST] >> 1;	//shift right one bit
			Register_file[DST] |= (Register_file[DST] & BIT_FOURTEEN) > 0 ? BIT_FIFTEEN : 0;	//set msb if have sign
		}
		break;
	}
	case BIC:	//Opcode = 0111 0010 (BIC)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2);	//do BIT operation
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 & ~data2);	//do BIT operation
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case RRC:	//Opcode = 0111 0011 (RRC)
	{
		unsigned char carry = GET_CARRY(PSW);
		(Register_file[DST] & BIT_ONE) > 0 ? PSW |= 1 : PSW &= ~1;	//set lsb to carry
		if (W_B > 0)	//if processing byte size data
		{
			unsigned short LSByte = Register_file[DST] & LOWER_BYTE;	//get lowbyte of destination register
			Register_file[DST] &= HIGHER_BYTE;	//clear loybyte of destination register
			LSByte = LSByte >> 1;	//shift right one bit
			LSByte |= carry > 0 ? BIT_SEVEN : 0;	//set msb if have sign
			Register_file[DST] |= LSByte;	//store back to register
		}
		else	//if processing word size data
		{
			Register_file[DST] = Register_file[DST] >> 1;	//shift right one bit
			Register_file[DST] |= carry > 0 ? BIT_FIFTEEN : 0;	//set msb if have sign
		}
		break;
	}
	case BIS:	//Opcode = 0111 0100 (BIS)
	{
		if (W_B > 0)	//if processing byte size data
		{
			unsigned char data1 = (unsigned char)Register_file[DST];	//get data from reigister[DST]
			unsigned char data2 = (unsigned char)(R_C > 0 ? const_table[SRC] : Register_file[SRC]);	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 | data2);	//do AND operation
			write_byte_to_dst(Register_file[DST], (unsigned char)result);	//load result to register
			ModifyStatusFlags(result, data1, data2, BYTE_CARRY_BIT, BYTE_SIGN_BIT);	//update psw register
		}
		else	//if processing word size data
		{
			unsigned short data1 = Register_file[DST];	//get data from reigister[DST]
			unsigned short data2 = R_C > 0 ? const_table[SRC] : Register_file[SRC];	//get data from const table [SRC] or register[SRC]
			unsigned int result = (data1 | data2);	//do AND operation
			Register_file[DST] = (unsigned short)result;	//load result to register
			ModifyStatusFlags(result, data1, data2, WORD_CARRY_BIT, WORD_SIGN_BIT);	//update psw register
		}
		break;
	}
	case SWPB:	//Opcode = 0111 0101 (SWPB)
	{
		unsigned short low_byte = Register_file[DST] << 8;	//get low byte of register and shift to high byte
		Register_file[DST] = Register_file[DST] >> 8;	//shift high byte data to low byte
		Register_file[DST] |= low_byte;	//load low byte data to high byte
		break;
	}
	case MOV:	//Opcode = 0111 0110 (MOV)
	{
		if (W_B > 0)	//if processing byte size data
			write_byte_to_dst(Register_file[DST], (unsigned char)Register_file[SRC]);	//copy data from src to dst
		else
			Register_file[DST] = Register_file[SRC];	//copy data from src to dst
		break;
	}
	case SXT:	//Opcode = 0111 0111 (SXT)
	{
		Register_file[DST] |= (Register_file[DST] & BIT_SEVEN) > 0 ? HIGHER_BYTE : 0;	//if seventh byte set, do sign extension
		break;
	}
	case SWAP:	//Opcode = 0111 1000 (SWAP)
	{
		unsigned short tmp_reg = Register_file[DST];	//save DST register value to a temporary variable
		Register_file[DST] = Register_file[SRC];	//copy source value to destination
		Register_file[SRC] = tmp_reg;	//copy temporary value to source
		break;
	}
	default:	//Error
	{
		std::cout << "\nOpcode has mistake\n";
		break;
	}
	}
}

//check interrput to emulate interrupt
void CPU::check_interrupt()
{
	//check pending interrupt(s) and add to interrupt queue
	std::map<unsigned char /*priority*/, unsigned char /*device number*/> dev_interrupt;
	for (unsigned char dev_number = 0; dev_number < 8; dev_number++)	//check each device's control/status register
	{
		unsigned short LByte_CSR = m_mem.m_memory.byte_mem[dev_number * 2];
		unsigned short clear_dba = LByte_CSR & (~CSR_DBA);
		if ((LByte_CSR&CSR_DBA) > 0)	//if data need to process
		{
			if ((LByte_CSR&CSR_IE) > 0)	//if interrupt generated
			{
				//get device priority
				unsigned char dev_psw = m_mem.m_memory.byte_mem[GET_DEV_VECTOR_ADDR(dev_number)];
				unsigned char dev_priority = GET_PRIORITY(dev_psw);
				dev_interrupt[dev_priority] = dev_number;//add to interrupt map
				m_mem.m_memory.byte_mem[dev_number * 2] = (unsigned char)clear_dba;	//clear DBA
			}
			if ((LByte_CSR&CSR_IO) == 0)	//if output device need to output data
			{
				unsigned short CSR = m_mem.m_memory.word_mem[dev_number];	//get device's CSR
				output_data_info info(GET_DATA(CSR), dev_number);
				unsigned int output_time = m_clock + device_process_time[dev_number];	//get output time
				output_list[output_time].emplace_back(info);	//add to list of output data
				m_mem.m_memory.byte_mem[dev_number * 2] = (unsigned char)clear_dba;	//clear DBA
			}
		}
	}
	if (dev_interrupt.size() > 0)
		interrput_queue.emplace_back(dev_interrupt);

	//output data tooutput file
	while (output_list.size() > 0 && (output_list.begin()->first <= m_clock))	//if there is data need write to to output file now
	{
		unsigned int time = output_list.begin()->first;
		for (size_t i = 0; i < output_list[time].size(); i++)
		{
			fout << "at time " << m_clock << ", device " << output_list[time].front().device_num << " output " << output_list[time].front().data << std::endl;	//output the data to output file
			output_list[time].pop_front();	//pop this output_data_info from the queue
		}
		output_list.erase(time);	//pop the queue of this time from output_list
	}

	//process pending interrupt(s)
	if (interrput_queue.size() > 0)	//if there is/are interrupt need to be process
	{
		std::deque<std::map<unsigned char /*priority*/, unsigned char /*device number*/>>::iterator it = interrput_queue.begin();	//get the earliest interrupt map in the queue

		unsigned char dev_priority = it->rbegin()->first;	//get the highest device's priority (the last one in the map, map automically sort element based on key value(priority))
		if (dev_priority > GET_PRIORITY(PSW))	//if device's priority > current CPU's priority, process interrupt
		{
			unsigned char dev_num = it->rbegin()->second;	//get device number
			push_to_stack(PROGRAM_COUNTER);	//push program counter to stack
			push_to_stack(PSW);	//push PSW to stack
			push_to_stack(LINK_REGISTER);	//push LR to stack

			//the handler's PSW becomes the current PSW
			m_cache.cache(GET_DEV_VECTOR_ADDR(dev_num), MDR, READ);
			PSW = MDR;

			//PC points to ISR
			m_cache.cache(GET_DEV_VECTOR_ADDR(dev_num) + 2, MDR, READ);
			PROGRAM_COUNTER = MDR;

			LINK_REGISTER = 0xffff;	//set LR to #$ffff

			it->erase(dev_priority);	//erase the pending interrupt from interrupt map of a time point
			if (it->size() == 0)	//if the interrupt map of a time point is empty, erase this map from interrupt queue
				interrput_queue.pop_front();
		}
	}
}

//get current priority
unsigned char CPU::get_current_priority()
{
	return GET_PRIORITY(PSW);
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

//return relative offset value from instruction (LDR STR)
short CPU::get_relative_offset()
{
	short offset = RELATIVE_OFFSET;	//get offset
	offset += (IR & BIT_TWELVE) > 0 ? RELATIVE_SIGN_EXTENSION : 0;	//check offset sign and do offset sign extend
	return offset;
}

//return offset value from instruction	(BEQ/BZ to BAL)
short CPU::get_offset()
{
	short offset = BRANCH_OFFSET;	//shift offset to right 1 bit
	offset += BRANCH_OFFSET_SIGN > 0 ? BRANCH_OFFSET_SIGN_EXTENSION : 0;	//offset sign extend
	return offset;
}

//update psw register
void CPU::ModifyStatusFlags(unsigned int result, unsigned int DST_Data, unsigned int SRC_Data, unsigned int carry_bit, unsigned int sign_bit)
{
	//indicate setting carry flag
	if ((result&carry_bit) > 0)	//if has carry
	{
		PSW |= GET_CARRY(PSW);	//set carry
		result -= carry_bit;	//unset carry bit from zero checking
	}
	else	//if no carry
		PSW &= ~GET_CARRY(PSW);	//clear carry

	//indicate setting overflow flag
	if ((DST_Data&sign_bit) == (SRC_Data&sign_bit))	//if source and destination have same sign
	{
		if ((DST_Data&sign_bit) != (result&sign_bit))	//if result and source have different sign
			PSW |= OVER_FLOW;	//set overflow flag
		else //clear overflow
			PSW &= ~OVER_FLOW;	//clear overflow flag
	}
	else //clear overflow
		PSW &= ~OVER_FLOW;	//clear overflow flag

	//indicate setting negative flag
	if ((result&sign_bit) > 0)	//if result is negative
		PSW |= NEGATIVE;	//set carry
	else //clear negative
		PSW &= ~NEGATIVE;	//clear negative flag

	//indicate setting zero flag
	if (result == 0)
		PSW |= ZERO;	//set zero
	else //clear zero
		PSW &= ~ZERO;	//clear zero flag
}

//update psw register
void CPU::ModifyStatusFlags_dec(unsigned int result, unsigned int DST_Data, unsigned int SRC_Data, unsigned int carry_bit)
{
	//indicate setting carry flag
	if ((result/ carry_bit) > 0)	//if bit 5 large than 0, so has carry
	{
		PSW |= CARRY;	//set carry
		result -= carry_bit;	//unset carry bit from zero checking
	}
	else	//if no carry
		PSW &= ~CARRY;	//clear carry

	PSW &= ~OVER_FLOW;	//clear overflow flag
	PSW &= ~NEGATIVE;	//clear negative flag

	//indicate setting zero flag
	if (result == 0)
		PSW |= ZERO;	//set zero
	else //clear zero
		PSW &= ~ZERO;	//clear zero flag
}

void CPU::push_to_stack(unsigned short data_to_push)	//push a data to stack pointer
{
	MDR = data_to_push;
	STACK_POINTER -= 2;
	m_cache.cache(STACK_POINTER, MDR, WRITE);
}

void CPU::pull_from_stack(unsigned short& data_to_pull)
{
	m_cache.cache(STACK_POINTER, MDR, READ);
	STACK_POINTER += 2;
	data_to_pull = MDR;
}



//routine after return from interrupt
void CPU::return_from_interrupt()
{
	pull_from_stack(LINK_REGISTER);	//pull LR
	pull_from_stack(PSW);	//pull PSW
	pull_from_stack(PROGRAM_COUNTER);	//pull PC
}

//load word size data to DST register
void CPU::load_word()
{
	MAR = Register_file[SRC];	//load memory address stored in register to MAR
	m_cache.cache(MAR, MDR, READ);	//access memory
	Register_file[DST] = MDR;	//load data from MDR to DST register
}

//load byte size data to DST register
void CPU::load_byte()
{
	MAR = Register_file[SRC];	//load memory address stored in register to MAR
	m_cache.cache(MAR, MDR, READ, BYTE);	//access memory
	write_byte_to_dst(Register_file[DST], (unsigned char)MDR);	//load data from MDR to DST register
}

//load word size data to DST register
void CPU::store_word()
{
	MAR = Register_file[DST];	//load memory address stored in register to MAR
	MDR = Register_file[SRC];	//store data from DST register to MDR
	m_cache.cache(MAR, MDR, WRITE);	//access memory
}

//load byte size data to DST register
void CPU::store_byte()
{
	MAR = Register_file[DST];	//load memory address stored in register to MAR
	MDR = Register_file[SRC];	//store data from DST register to MDR
	m_cache.cache(MAR, MDR, WRITE, BYTE);	//access memory
}

int hex_to_dec(unsigned short data)
{
	return GET_1ST_FOUR_BITS(data) + GET_2ND_FOUR_BITS(data) * 10 + GET_3RD_FOUR_BITS(data) * 100 + GET_4TH_FOUR_BITS(data) * 1000;
}

unsigned short dec_to_hex(int data)
{
	int result_1 = data % 10;
	int result_10 = (data / 10) % 10;
	int result_100 = (data / 100) % 10;
	int result_1000 = (data / 1000) % 10;
	return (unsigned short)(result_1 + (result_10 << 4) + (result_100 << 8) + (result_1000 << 12));
}