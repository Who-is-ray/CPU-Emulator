//Class of CPU
#pragma once
#include<string>

#define SIZE_OF_REGISTER	8
#define SIZE_OF_CONST_TABLE	8

class CPU
{
public:
	CPU(unsigned char* memory);	//constructor
	~CPU();	//destructor

	unsigned short get_register_val(int loc);	//get a specific register value
	void set_register_val(int address, unsigned short val);	//set a specific register value

	void fetch();	//CPU fetch function, emulate fetch routine
	void decode();	//CPU decode function, emulate decode routine
	void execute();	//CPU execute function, emulate execute routine

private:
	unsigned short Register_file[SIZE_OF_REGISTER] = {10};	//register file
	unsigned char* m_memory;	//memory pointer
	unsigned short MAR;	//memory address register
	unsigned short MDR;	//memory data register
	unsigned short IR;	//instruction register
	int CPU_clock;	//CPU clock
	unsigned const short const_table[SIZE_OF_CONST_TABLE] = { 0,1,2,4,8,32,48, 0xffff };	//constant table that store the constand value

	void bus(bool read_write, bool byte_word = 1);	//bus function
	void write_byte_to_dst(unsigned short& dst, unsigned char data);	//write a byte size data to destination lower byte
	unsigned char get_DST();	//return DST info from instruction
	unsigned char get_SRC();	//return SRC info from instruction
	short get_relative_offset();	//return relative offset value from instruction (LDR STR)
	unsigned char get_data();	//return data value from instruction	(MOVL, MOVLZ, MOVH)
	short get_offset();	//return offset value from instruction	(BEQ/BZ to BAL)
};