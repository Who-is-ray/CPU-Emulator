//Class of CPU

#pragma once
#include<string>

#define SIZE_OF_REGISTER

class CPU
{
public:
	CPU();
	~CPU();

private:
	std::string Register[SIZE_OF_REGISTER];
};

