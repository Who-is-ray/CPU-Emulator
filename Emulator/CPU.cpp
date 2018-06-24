#include "CPU.h"

#define SIZE_OF_REGISTER 8
#define SIZE_OF_MEMORY 65536	//size of memory

CPU::CPU(unsigned char* memory):m_memory(memory)
{
	Register_file = new unsigned short[SIZE_OF_REGISTER];
}


CPU::~CPU()
{
	delete Register_file;
}
