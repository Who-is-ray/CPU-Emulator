//Class of CPU
#pragma once
#include<string>

#define SIZE_OF_REGISTER	8
#define SIZE_OF_CONST_TABLE	8

class Memory;

class CPU
{
public:
	CPU(Memory& memory);	//constructor
	~CPU() {}	//destructor

	unsigned short get_register_val(int loc);	//get a specific register value
	void set_register_val(int address, unsigned short val);	//set a specific register value

	void fetch();	//CPU fetch function, emulate fetch routine
	void decode();	//CPU decode function, emulate decode routine
	void execute();	//CPU execute function, emulate execute routine

private:
	unsigned short Register_file[SIZE_OF_REGISTER] = {NULL};	//register file
	Memory& m_mem;	//memory pointer
	unsigned short MAR;	//memory address register
	unsigned short MDR;	//memory data register
	unsigned short IR;	//instruction register
	int CPU_clock;	//CPU clock
	unsigned const short const_table[SIZE_OF_CONST_TABLE] = { 0,1,2,4,8,32,48,0xffff };	//constant table that store the constand value

	void write_byte_to_dst(unsigned short& dst, unsigned char data);	//write a byte size data to destination lower byte
	unsigned char DST_EA();	//return DST info from instruction
	unsigned char SRC_EA();	//return SRC info from instruction
	short get_relative_offset();	//return relative offset value from instruction (LDR STR)
	unsigned char get_data();	//return data value from instruction	(MOVL, MOVLZ, MOVH)
	short get_offset();	//return offset value from instruction	(BEQ/BZ to BAL)
	void ModifyStatusFlags(unsigned int result, unsigned int data1, unsigned int data2, unsigned int carry_bit, unsigned int sign_bit);	//updat psw register
};