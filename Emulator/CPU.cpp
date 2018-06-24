#include "CPU.h"

#define SIZE_OF_REGISTER 8

CPU::CPU()
{
	Register_file = new short[SIZE_OF_REGISTER];
}


CPU::~CPU()
{
	delete Register_file;
}
