//Class of CPU
#pragma once
#include<string>

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
	unsigned short* Register_file;	//register file
	unsigned char* m_memory;	//memory pointer
	unsigned short MAR;	//memory address register
	unsigned short MDR;	//memory data register
	unsigned short IR;	//instruction register
	void bus(bool read_write, bool byte_word);	//bus function
	int CPU_clock;	//CPU clock
};