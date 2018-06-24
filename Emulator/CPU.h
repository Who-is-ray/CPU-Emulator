//Class of CPU

#pragma once
#include<string>

class Debugger;

class CPU
{
public:
	CPU(unsigned char* memory);
	~CPU();

private:
	unsigned short* Register_file;
	unsigned char* m_memory;
};

