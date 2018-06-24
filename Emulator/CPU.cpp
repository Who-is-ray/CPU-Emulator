#include "CPU.h"

#define SIZE_OF_REGISTER 8

//constructor
CPU::CPU(unsigned char* memory):m_memory(memory)
{
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